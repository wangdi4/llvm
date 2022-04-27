//===-----------DTransTypes.h - Type model for DTrans ---------------------===//
//
// Copyright (C) 2019-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

// This file is used for representing IR data types used by DTrans. DTrans
// analysis requires knowing the types of pointer objects to distinguish
// between %struct.bar* and %struct.foo*. With opaque pointers,
// llvm::PointerType objects no longer indicate the type of object pointed to.
// This also means that elements that may contain pointers, such as structures
// or function signatures need to be represented in a form that can maintain
// the type of pointers within them. The types in this file are used for
// representing these types, in a similar fashion to the llvm::Type class
// hierarchy prior to conversion to opaque pointers.

#if !INTEL_FEATURE_SW_DTRANS
#error DTransTypes.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSTYPES_H
#define INTEL_DTRANS_ANALYSIS_DTRANSTYPES_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/DerivedTypes.h"

#include <vector>

namespace llvm {
class LLVMContext;
class MDNode;

namespace dtransOP {

class DTransTypeManager;
class DTransType;

// This class stores information regarding a field member of a structure.
class DTransFieldMember {
public:
  DTransFieldMember(DTransType *DTType = nullptr) {
    if (DTType)
      addResolvedType(DTType);
  }

  // Add the type to the list of possible types for this field. Return 'true'
  // if this resulted in a change of information for this field. Because the
  // field types are being reconstructed from metadata, we allow for the
  // potential that conflicting information was seen when reconstructing the
  // type from the metadata by allowing for multiple types. DTrans safety checks
  // will need to handle this as a potential safety issue for the structure
  // containing this field.
  bool addResolvedType(DTransType *Ty) { return DTTypes.insert(Ty).second; }

  // Get the list of possible type for this field.
  const SmallPtrSetImpl<DTransType *> &getTypes() const { return DTTypes; }

  // Get the field type if decoding the metadata resulted in a unique type.
  DTransType *getType() const {
    if (DTTypes.size() != 1)
      return nullptr;
    return *DTTypes.begin();
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD void dump() const;
  LLVM_DUMP_METHOD void print(llvm::raw_ostream &OS, bool Brief = false) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  // The DTrans type the field corresponds to. Allow for multiple types
  // to detect errors when reconstructing from metadata.
  SmallPtrSet<DTransType *, 1> DTTypes;
};

// This is the base class for representing types within DTrans analysis. There
// will not be any direct instantiations of this class.
class DTransType {
public:
  // Definitions to support type inquiry through isa, cast, and dyn_cast
  enum DTransTypeID {
    DTransAtomicTypeID, // Used for a void or "first class llvm Type"
    DTransPointerTypeID,
    DTransStructTypeID,
    DTransArrayTypeID,
    DTransVectorTypeID,
    DTransFunctionTypeID
  };

protected:
  // This class is only constructed and destructed via a derived class.
  DTransType(DTransTypeID TID, LLVMContext &Ctx) : ID(TID), Ctx(Ctx) {}

  // Disallow copy, movement, and assignment operations because the
  // DTransTypeManager is keeping track of the constructed objects to perform
  // memory management on them.
  DTransType(const DTransType &) = delete;
  DTransType(DTransType &&) = delete;
  DTransType &operator=(const DTransType &) = delete;
  DTransType &operator=(DTransType &&) = delete;

  ~DTransType() = default;

public:
  // Get the derived object's type.
  DTransTypeID getTypeID() const { return ID; }

  // Convert the type to the equivalent llvm::Type. This is to support the
  // transition to using these types within some parts of DTrans while other
  // parts work with llvm::Type objects. This may be removed in the future
  // when all of DTrans is converted, and the llvm core type system is
  // solely using opaque pointers.
  llvm::Type *getLLVMType() const;

  // This is needed to support getLLVMType().
  llvm::LLVMContext &getContext() const { return Ctx; }

  // Return a metadata node that describes the type.
  MDNode *createMetadataReference() const;

  // Compare two DTrans types for equivalence.
  bool compare(const DTransType &Other) const;

  // Helper utilities that match frequently used methods of llvm::Type.
  bool isAtomicTy() const { return getTypeID() == DTransAtomicTypeID; }
  bool isPointerTy() const { return getTypeID() == DTransPointerTypeID; }
  bool isStructTy() const { return getTypeID() == DTransStructTypeID; }
  bool isArrayTy() const { return getTypeID() == DTransArrayTypeID; }
  bool isVectorTy() const { return getTypeID() == DTransVectorTypeID; }
  bool isFunctionTy() const { return getTypeID() == DTransFunctionTypeID; }
  bool isAggregateType() const { return isStructTy() || isArrayTy(); }

  // Get the number of fields of a structure, or elements of a sequential type.
  uint32_t getNumContainedElements() const;

  bool isIntegerTy() const {
    return isAtomicTy() && getLLVMType()->isIntegerTy();
  }
  bool isFloatingPointTy() const {
    return isAtomicTy() && getLLVMType()->isFloatingPointTy();
  }

  // Helper method that casts this object to a pointer type, and returns
  // the type pointed to. Derived object must be DTransPointer.
  DTransType *getPointerElementType() const;

  // Helper method that casts this object to an array type, and returns
  // the type stored in the array. Derived object must be DTransArray.
  DTransType *getArrayElementType() const;

  // Helper method that casts this object to a vector type, and returns
  // the type stored in the vector. Derived object must be DTransVectorType.
  DTransType *getVectorElementType() const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD void dump() const;

  // Print the type.
  // \p Detailed - When 'true', internals of structure types have their
  //               field types printed.
  LLVM_DUMP_METHOD void print(llvm::raw_ostream &OS,
                              bool Detailed = true) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  // ID to support type inquiry through isa, cast, and dyn_cast
  DTransTypeID ID;

  // The context is currently needed to support converting from this type system
  // representation back to llvm::Type types.
  LLVMContext &Ctx;
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static inline raw_ostream &operator<<(raw_ostream &OS,
                                      const DTransType &DTType) {
  DTType.print(OS, true);
  return OS;
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

static inline bool operator==(const DTransType &LHS, const DTransType &RHS) {
  return LHS.compare(RHS);
}

// This class is used to represent llvm first class types. These are types that
// do not have pointers, and do not contain references to other types which
// potentially could have pointers, and therefore will still be unique when
// there are opaque pointers. For example, this class is used for integer types,
// float types, or the void type. It may not be used for structures, arrays,
// vectors, functions, etc. because those types may contain pointers to other
// types.
class DTransAtomicType : public DTransType {
private:
  // Construction of the class may only be done by the DTransTypeManager class.
  // This is to ensure the DTransTypeManager can be responsible for all memory
  // allocations/deallocations of the types.
  friend class DTransTypeManager;
  explicit DTransAtomicType(llvm::Type *Ty)
      : DTransType(DTransAtomicTypeID, Ty->getContext()), LLVMType(Ty) {
    // This type encapsulates the single value types (integer types, floating
    // point types, etc) with the exception of pointer types. It is also used
    // for specialized types that do not represent aggregate types (void type,
    // metadata type and token type). Assert to ensure that this class does not
    // get used to represent a complex type (pointer, structure, array,
    // function, etc) that requires special handling by DTrans.
    assert((Ty->isIntegerTy() || Ty->isFloatingPointTy() || Ty->isVoidTy() ||
            Ty->isMetadataTy() || Ty->isTokenTy()) &&
           "Atomic type must be first class type");
  }

  // For now, destruction is restricted to the DTransTypeManager class, as well,
  // since it is keeping a table of objects, and we are not maintaining
  // reference counting to the objects. The DTransTypeManager class will delete
  // all objects at once. This may change in the future, if there is a need to
  // create the types outside of the DTransTypeManager.
  ~DTransAtomicType() {}

public:
  // Factory method to construct the object that will be owned by the
  // DTransTypeManager.
  static DTransAtomicType *get(DTransTypeManager &TM, llvm::Type *Ty);

  static inline bool classof(const DTransType *TI) {
    return TI->getTypeID() == DTransType::DTransAtomicTypeID;
  }

  bool compare(const DTransAtomicType &Other) const {
    return LLVMType == Other.LLVMType;
  }

  llvm::Type *getLLVMType() const { return LLVMType; }
  bool isVoidTy() const { return LLVMType->isVoidTy(); }
  bool isMetadataTy() const { return LLVMType->isMetadataTy(); }
  bool isIntegerTy() const { return LLVMType->isIntegerTy(); }
  bool isFloatingPointTy() const { return LLVMType->isFloatingPointTy(); }

  // Return a metadata node that describes the type.
  MDNode *createMetadataReference(unsigned PtrLevel = 0) const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD void print(llvm::raw_ostream &OS) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  // This object directly maps to a llvm::Type object.
  llvm::Type *LLVMType;
};

// This class is used to represent pointer types by storing the type of
// object being "pointed-to".
class DTransPointerType : public DTransType {
private:
  // Construction of the class may only be done by the DTransTypeManager class.
  // This is to ensure the DTransTypeManager can be responsible for all memory
  // allocations/deallocations of the types.
  friend class DTransTypeManager;
  explicit DTransPointerType(LLVMContext &Ctx, DTransType *PointeeTy)
      : DTransType(DTransPointerTypeID, Ctx), PointeeType(PointeeTy) {
    assert(PointeeTy && "PointeeType cannot be nullptr");
  }

  // Destruction is restricted to the DTransTypeManager class, as well, since it
  // is keeping a table of objects.
  ~DTransPointerType() {
    // Nothing to delete. We don't delete the PointerType member, because that
    // is owned by the type manager class.
  }

public:
  // Factory method to construct the object that will be owned by the
  // DTransTypeManager.
  static DTransPointerType *get(DTransTypeManager &TM, DTransType *PointeeTy);

  static inline bool classof(const DTransType *TI) {
    return TI->getTypeID() == DTransType::DTransPointerTypeID;
  }

  bool compare(const DTransPointerType &Other) const {
    return PointeeType->compare(*Other.PointeeType);
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD void print(llvm::raw_ostream &OS) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  DTransType *getPointerElementType() const { return PointeeType; }

  // TODO: When the opaque pointers are enabled, this will need to be changed to
  // just return an opaque pointer.
  llvm::Type *getLLVMType() const {
    return getPointerElementType()->getLLVMType()->getPointerTo();
  }

  // Return a metadata node that describes the type.
  MDNode *createMetadataReference() const;

private:
  // The "pointed-to" object type.
  DTransType *PointeeType;
};

// A composite type is used for types which can contain more than a single
// element, either of the same type or of varying types. This class is for
// compatibility with llvm::CompositeType.
class DTransCompositeType : public DTransType {
protected:
  // This may only be constructed via a derived class
  explicit DTransCompositeType(DTransType::DTransTypeID ID, LLVMContext &Ctx)
      : DTransType(ID, Ctx) {}

  ~DTransCompositeType() {}

public:
  static inline bool classof(const DTransType *TI) {
    return TI->getTypeID() == DTransType::DTransStructTypeID ||
           TI->getTypeID() == DTransType::DTransArrayTypeID ||
           TI->getTypeID() == DTransType::DTransVectorTypeID;
  }

  // Returns true if \p Idx is valid for composite type
  bool indexValid(unsigned Idx) const;
};

// Represents a llvm::StructType
class DTransStructType : public DTransCompositeType {
private:
  // Construction of the class may only be done by the DTransTypeManager class.
  // This is to ensure the DTransTypeManager can be responsible for all memory
  // allocations/deallocations of the types.
  friend class DTransTypeManager;

  // Create a representation for the llvm::StructType, and populate the body
  // with the \p FieldTypes. This can only be used for non-literal structure
  // types.
  DTransStructType(llvm::StructType *Ty, ArrayRef<DTransFieldMember> FieldTypes)
      : DTransCompositeType(DTransStructTypeID, Ty->getContext()), LLVMType(Ty),
        Name(Ty->getName()), IsPacked(Ty->isPacked()) {
    assert(Ty->isStructTy() &&
           "StructType type must be based on llvm::StructType");
    std::copy(FieldTypes.begin(), FieldTypes.end(), std::back_inserter(Fields));
  }

  // Constructor for a literal struct type, with known field types.
  // This type does not get associated with the corresponding LLVMType because
  // without a name for the type, it's possible that multiple types would match
  // when there are opaque pointers.
  DTransStructType(LLVMContext &Ctx, ArrayRef<DTransFieldMember> FieldTypes,
                   bool IsPacked = false)
      : DTransCompositeType(DTransStructTypeID, Ctx), LLVMType(nullptr),
        IsPacked(IsPacked), IsLiteral(true) {
    std::copy(FieldTypes.begin(), FieldTypes.end(), std::back_inserter(Fields));
  }

  // Generic constructor for a struct type, where field types are not known yet,
  // so creates placeholders for the field members.
  DTransStructType(LLVMContext &Ctx, llvm::StructType *Ty, std::string Name,
                   unsigned NumFields, bool IsOpaque = false,
                   bool IsLiteral = false, bool IsPacked = false)
      : DTransCompositeType(DTransStructTypeID, Ctx), LLVMType(Ty), Name(Name),
        IsPacked(IsPacked), IsOpaque(IsOpaque), IsLiteral(IsLiteral) {
    assert(!(IsOpaque && IsLiteral) && "Cannot be both opaque and literal");
    assert(!(IsOpaque && NumFields) &&
           "Opaque type not allowed to have fields");
    assert(!(Ty && IsLiteral) &&
           "Should not associate literal type with llvm type");
    if (NumFields)
      Fields.resize(NumFields);
  }

public:
  // Destructor needs to be visible to use unique_ptr internally.
  ~DTransStructType() {
    // Nothing to delete. All pointers are owned by the DTransTypeManager.
  }

  // Create a DTransStructType from an existing llvm::StructType. Field members
  // that do not involve pointer types will be populated. Any fields that
  // involve pointer types will be set to nullptr, and need to be set with
  // addResolvedType after the type is created.
  static DTransStructType *get(DTransTypeManager &TM, llvm::StructType *Ty);

  static inline bool classof(const DTransType *TI) {
    return TI->getTypeID() == DTransType::DTransStructTypeID;
  }

  // Because the type is being reconstructed based on metadata coming from
  // multiple files, information is stored to indicate that the type may not be
  // valid due to conflicts being detected during type recovery. These functions
  // are there to note that errors occurred.
  bool getReconstructError() const { return ReconstructError; }
  void setReconstructError() { ReconstructError = true; }

  bool hasName() const { return !Name.empty(); }
  StringRef getName() const { return Name; }
  bool isPacked() const { return IsPacked; }
  bool isOpaque() const { return IsOpaque; }
  bool isLiteralStruct() const { return IsLiteral; }

  // Grow the number fields needed to be represented in the structure. This is
  // to accommodate the case where conflicts are encountered with trying to
  // reconstruct the type from metadata.
  void resizeFieldCount(unsigned NewFieldCount) {
    assert(NewFieldCount > getNumFields() && "Only addition of fields allowed");
    Fields.resize(NewFieldCount);
    if (IsOpaque)
      IsOpaque = false;
  }

  // Set the body of an opaque structure type.
  void setBody(ArrayRef<DTransType *> Fields) {
    assert(IsOpaque && "Adding a body requires structure type to be opaque");
    size_t FieldCount = Fields.size();
    IsOpaque = false;
    if (FieldCount == 0)
      return;

    resizeFieldCount(FieldCount);
    unsigned Idx = 0;
    for (auto *FieldType : Fields)
      getField(Idx++).addResolvedType(FieldType);
  }

  bool compare(const DTransStructType &Other) const {
    // For non-literal structures, we will assume for now that structures with
    // the same name are equal, because otherwise walking the elements member by
    // member needs to track a list of visited elements to avoid an infinite
    // recursion. Literal types could have multiple instances created because
    // they are created on the fly, so for those which recurse into the fields,
    // it should not be possible to form a recursive chain that leads back to
    // this type for them.
    if (!isLiteralStruct())
      return Name == Other.Name;

    unsigned NumFields1 = getNumFields();
    unsigned NumFields2 = Other.getNumFields();
    if (NumFields1 != NumFields2)
      return false;

    for (auto P : zip_first(getFields(), Other.getFields())) {
      auto It1 = std::get<0>(P);
      auto It2 = std::get<1>(P);

      // We should not have a way for literal types to be created with multiple
      // types for a field, assert to be sure.
      assert(
          It1.getTypes().size() == 1 &&
          "Literal types should not have multiple possible field resolutions");
      assert(
          It2.getTypes().size() == 1 &&
          "Literal types should not have multiple possible field resolutions");

      auto *FieldType1 = *It1.getTypes().begin();
      auto *FieldType2 = *It2.getTypes().begin();
      if (!FieldType1->compare(*FieldType2))
        return false;
    }

    return true;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD void print(llvm::raw_ostream &OS, bool Brief = false) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  const SmallVectorImpl<DTransFieldMember> &getFields() const { return Fields; }

  const DTransFieldMember &getField(size_t N) const {
    assert(N < Fields.size() && "Field index out of bounds");
    return Fields[N];
  }

  DTransFieldMember &getField(size_t N) {
    assert(N < Fields.size() && "Field index out of bounds");
    return Fields[N];
  }

  // If the field was resolved to be a single type, return it. Otherwise,
  // nullptr.
  DTransType *getFieldType(size_t N) {
    if (getReconstructError())
      return nullptr;

    auto &Field = Fields[N];
    auto &ResolvedTypes = Field.getTypes();
    if (ResolvedTypes.size() != 1)
      return nullptr;

    return *ResolvedTypes.begin();
  }

  unsigned getNumFields() const { return Fields.size(); }

  // Convert to an existing LLVMType. If no LLVMType type exists for the
  // structure, returns nullptr. This should be ok, because this is meant for
  // being able to match up to types within the LLVM IR, so if the type doesn't
  // exist within the IR, we will not care about it.
  llvm::Type *getLLVMType() const {
    if (LLVMType)
      return LLVMType;

    assert(isLiteralStruct() && "Expected literal struct");
    SmallVector<llvm::Type *, 8> Types;
    for (auto &F : Fields) {
      llvm::Type *Ty = (*F.getTypes().begin())->getLLVMType();
      Types.push_back(Ty);
    }

    auto *LitSt = llvm::StructType::get(getContext(), Types, IsPacked);
    return LitSt;
  }

  // Return a metadata node that describes the type.
  MDNode *createMetadataReference(unsigned PtrLevel = 0) const;

  // Return a metadata node that is used to describe the body of the structure.
  MDNode *createMetadataStructureDescriptor() const;

  using DTransFieldMemberContainerTy = SmallVector<DTransFieldMember, 16>;
  using FieldsIterator = DTransFieldMemberContainerTy::iterator;
  using FieldsConstIterator = DTransFieldMemberContainerTy::const_iterator;
  iterator_range<FieldsIterator> elements() {
    return make_range(Fields.begin(), Fields.end());
  }
  iterator_range<FieldsConstIterator> elements() const {
    return make_range(Fields.begin(), Fields.end());
  }

private:
  // The corresponding LLVMType for non-literal structures, if one exists. We do
  // not map literal structures to llvm types because when there are opaque
  // pointers we could have multiple DTransStructTypes that represent the
  // literal struct, {ptr, ptr}. It's also possible this will be nullptr
  // depending on the metadata encoding because some structures could be
  // eliminated between the front-end and the backend.
  llvm::StructType *LLVMType;

  // Because LLVMType may be nullptr, store the name of the structure this type
  // originally referred to.
  std::string Name;

  // Members of the structure.
  DTransFieldMemberContainerTy Fields;

  // Various attributes of the structure being represented.
  // TODO: IsPacked may not be needed, include it for now.
  bool IsPacked = false;
  bool IsOpaque = false;
  bool IsLiteral = false;

  // Indicates whether conflicting information was seen when using the metadata
  // to rebuild this type.
  bool ReconstructError = false;
};

// A sequential type is used as a base class for Arrays and Vectors. This is to
// provide compatibility with the interface routines provided by
// llvm::SequentialType.
class DTransSequentialType : public DTransCompositeType {
protected:
  // This may only be constructed via a derived class.
  DTransSequentialType(DTransType::DTransTypeID ID, LLVMContext &Ctx,
                       DTransType *DTType, uint64_t Num)
      : DTransCompositeType(ID, Ctx), DTType(DTType), Num(Num) {
    assert(DTType && "Sequential type must not be nullptr for DTrans type");
  }

public:
  static inline bool classof(const DTransType *TI) {
    return TI->getTypeID() == DTransType::DTransArrayTypeID ||
           TI->getTypeID() == DTransType::DTransVectorTypeID;
  }

  uint64_t getNumElements() const { return Num; }

  // Note, this function is being defined with the same interface as
  // llvm::CompositeType::getTypeAtIndex to enable compatibility during the
  // migration to using DTransTypes, but within DTrans it differs by being
  // placed within the SequentialType derivation because it may be possible that
  // a structure type does not contain a unique type if the type recovery fails
  // to uniquely recovery the structure.
  // TODO: Should to either move this to DTransCompositeType if some sensible
  // return value can be determined or remove the unused parameter.
  DTransType *getTypeAtIndex(uint64_t) const { return DTType; }
  DTransType *getElementType() const { return DTType; }

  llvm::Type *getLLVMType() const {
    if (getTypeID() == DTransType::DTransArrayTypeID)
      return ArrayType::get(getTypeAtIndex(0)->getLLVMType(), getNumElements());
    else
      return VectorType::get(getTypeAtIndex(0)->getLLVMType(), getNumElements(),
                             /*Scalable=*/false);
  }

  // Return a metadata node that describes the type.
  MDNode *createMetadataReference() const;

  bool compare(const DTransSequentialType &Other) const {
    return getNumElements() == Other.getNumElements() &&
           getTypeAtIndex(0)->compare(*Other.getTypeAtIndex(0));
  }

private:
  DTransType *DTType;
  uint64_t Num;
};

// Used to represent a llvm::ArrayType
class DTransArrayType : public DTransSequentialType {
private:
  // Construction of the class may only be done by the DTransTypeManager class.
  // This is to ensure the DTransTypeManager can be responsible for all memory
  // allocations/deallocations of the types.
  friend class DTransTypeManager;
  DTransArrayType(LLVMContext &Ctx, DTransType *DTType, uint64_t Num)
      : DTransSequentialType(DTransType::DTransArrayTypeID, Ctx, DTType, Num) {}

  ~DTransArrayType() {}

public:
  // Factory method to construct the object that will be owned by the
  // DTransTypeManager.
  static DTransArrayType *get(DTransTypeManager &TM, DTransType *DTType,
                              uint64_t Num);

  static inline bool classof(const DTransType *TI) {
    return TI->getTypeID() == DTransType::DTransArrayTypeID;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD void print(llvm::raw_ostream &OS) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
};

// Used to represent a llvm::VectorType
class DTransVectorType : public DTransSequentialType {
private:
  // Construction of the class may only be done by the DTransTypeManager class.
  // This is to ensure the DTransTypeManager is responsible for all memory
  // allocations/deallocations of the types.
  friend class DTransTypeManager;
  DTransVectorType(LLVMContext &Ctx, DTransType *DTType, uint64_t Num)
      : DTransSequentialType(DTransType::DTransVectorTypeID, Ctx, DTType, Num) {
  }

  ~DTransVectorType() {}

public:
  // Factory method to construct the object that will be owned by the
  // DTransTypeManager.

  static DTransVectorType *get(DTransTypeManager &TM, DTransType *DTType,
                               uint64_t Num);

  static inline bool classof(const DTransType *TI) {
    return TI->getTypeID() == DTransType::DTransVectorTypeID;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD void print(llvm::raw_ostream &OS) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
};

// This class contains a description of the function signature for any function
// pointers.
class DTransFunctionType : public DTransType {
private:
  // Construction of the class may only be done by the DTransTypeManager class.
  // This is to ensure the DTransTypeManager is responsible for all memory
  // allocations/deallocations of the types.
  friend class DTransTypeManager;
  DTransFunctionType(LLVMContext &Ctx, size_t NumArgs, bool IsVarArg)
      : DTransType(DTransFunctionTypeID, Ctx), NumArgs(NumArgs),
        IsVarArg(IsVarArg) {
    // Allocate elements for storing the return type and the arguments
    ContainedTypes.resize(NumArgs + 1);
  }

  ~DTransFunctionType() {}

public:
  static inline bool classof(const DTransType *TI) {
    return TI->getTypeID() == DTransType::DTransFunctionTypeID;
  }

  // Compare another function type with this one. If one of the types has a
  // nullptr for a parameter or return type, and the other doesn't, treat
  // the return/parameter type as matching (This is to eventually allow for
  // partially resolved indirect function call types, where some parameter types
  // have been determined, but all parameters have not been resolved yet).
  bool compare(const DTransFunctionType &Other) const {
    if (getNumArgs() != Other.getNumArgs())
      return false;

    if (isVarArg() ^ Other.isVarArg())
      return false;

    auto *R1 = getReturnType();
    auto *R2 = Other.getReturnType();
    bool HaveR1 = R1 != nullptr;
    bool HaveR2 = R2 != nullptr;
    // If both have types, compare them. Otherwise ensure neither has types.
    if (HaveR1 ^ HaveR2)
      return false;

    if (R1)
      if (!R1->compare(*R2))
        return false;

    for (size_t ArgNum = 1; ArgNum <= NumArgs; ++ArgNum) {
      auto *A1 = ContainedTypes[ArgNum];
      auto *A2 = Other.ContainedTypes[ArgNum];

      bool HaveA1 = A1 != nullptr;
      bool HaveA2 = A2 != nullptr;
      if (HaveA1 ^ HaveA2)
        return false;

      if (A1)
        if (!A1->compare(*A2))
          return false;
    }

    return true;
  }

  llvm::Type *getLLVMType() const {
    // Any unknown pointer type fields will be substituted with i8* when
    // producing the llvm::Type. This is to allow for the possibility during
    // type recovery a function type can be created for something that is known
    // to be a pointer to a function, even if the individual parameter types or
    // return type is not known at the time.
    auto *Int8PtrTy = llvm::Type::getInt8PtrTy(getContext());

    llvm::Type *FuncRetTy = Int8PtrTy;
    DTransType *RetTy = getReturnType();
    if (RetTy)
      FuncRetTy = RetTy->getLLVMType();

    SmallVector<Type *, 8> DataTypes;
    for (auto *Arg : args()) {
      if (Arg)
        DataTypes.push_back(Arg->getLLVMType());
      else
        DataTypes.push_back(Int8PtrTy);
    }

    return FunctionType::get(FuncRetTy, makeArrayRef(DataTypes), isVarArg());
  }

  // Return a metadata node that describes the type.
  MDNode *createMetadataReference() const;

  static DTransFunctionType *get(DTransTypeManager &TM, DTransType *DTRetTy,
                                 SmallVectorImpl<DTransType *> &ParamTypes,
                                 bool IsVarArg);

  // Create a DTransFunctionType, without known types.
  static DTransFunctionType *get(DTransTypeManager &TM, unsigned NumArgs,
                                 bool IsVarArg);

  DTransType *getReturnType() const { return ContainedTypes[0]; }

  // Try to set the return type. Return 'true', if type changed.
  bool setReturnType(DTransType *RetTy) {
    if (!RetTy)
      return false;

    DTransType *CurrentTy = getReturnType();
    if (CurrentTy == RetTy)
      return false;

    assert(CurrentTy == nullptr && "Conflicting types for return type");
    ContainedTypes[0] = RetTy;
    return true;
  }

  ArrayRef<DTransType *> args() const {
    auto *Beg = ContainedTypes.begin();
    ++Beg;
    return ArrayRef<DTransType *>(Beg, ContainedTypes.end());
  };

  // Get an argument type.
  // Arg numbering is from 0..NumArgs-1
  DTransType *getArgType(unsigned ArgNum) const {
    assert(ArgNum < NumArgs && "Arg index out of range");
    // The return type is stored at index 0.
    return ContainedTypes[ArgNum + 1];
  }

  // Try to set the argument type. Return 'true', if type changed.
  // Arg numbering is from 0..NumArgs-1.
  bool setArgType(unsigned ArgNum, DTransType *Ty) {
    assert(ArgNum < NumArgs && "Arg index out of range");
    if (!Ty)
      return false;

    DTransType *CurrentTy = getArgType(ArgNum);
    if (CurrentTy == Ty)
      return false;

    assert(CurrentTy == nullptr && "Conflicting types for arg type");
    ContainedTypes[ArgNum + 1] = Ty;
    return true;
  }

  bool isVarArg() const { return IsVarArg; }
  size_t getNumArgs() const { return NumArgs; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD void print(llvm::raw_ostream &OS) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  // Description of the return type (stored at index 0), and the parameter types
  // (stored at index 1..N)
  SmallVector<DTransType *, 8> ContainedTypes;

  size_t NumArgs;
  bool IsVarArg;
};

// This class is used to track DTransFunctionType objects so that only a single
// instance is created for each unique function type.
class DTransFunctionTypeNode : public FoldingSetNode {
public:
  DTransFunctionTypeNode(DTransFunctionType *FTy) : FTy(FTy) {}

  // Used by the FoldingSet template instantiation of this type to generate a
  // unique fingerprint that corresponds to the 'DTransFunctionType' object
  // stored.
  void Profile(FoldingSetNodeID &ID) const {
    // The ID, is just a concatenation of these values to get a unique value for
    // the function signature of the DTransFunctionType object.
    ID.AddPointer(FTy->getReturnType());
    ID.AddInteger(FTy->getNumArgs());
    for (auto CurParamTy : FTy->args())
      ID.AddPointer(CurParamTy);
    ID.AddBoolean(FTy->isVarArg());
  }

  // Generate the fingerprint value for a DTransFunctionType that would be
  // created if a DTransFunctionType were created with the specified types.
  static void generateProfile(DTransType *DTRetTy,
                              ArrayRef<DTransType *> ParamTypes,
                              bool IsVarArg, FoldingSetNodeID &ID) {
    // This must match the logic of the 'Profile' member so that the same
    // fingerprint will be generated once a DTransFunctionType object is
    // created.
    ID.AddPointer(DTRetTy);
    ID.AddInteger(ParamTypes.size());
    for (auto CurParamTy : ParamTypes)
      ID.AddPointer(CurParamTy);
    ID.AddBoolean(IsVarArg);
  }

  DTransFunctionType *getFunctionType() const { return FTy; }

private:
  DTransFunctionType *FTy;
};

// This class is used for keeping track of what types exist and owns the memory
// for all the types
class DTransTypeManager {
public:
  DTransTypeManager(LLVMContext &Ctx) : Ctx(Ctx) {}
  ~DTransTypeManager();

  // Disallow copying because the class owns all the DTransType pointers.
  // Disallow movement. The safety analyzer should create the object, and let
  // the other passes use a reference to it.
  DTransTypeManager(const DTransTypeManager &) = delete;
  DTransTypeManager(DTransTypeManager &&) = delete;
  DTransTypeManager &operator=(const DTransTypeManager &) = delete;
  DTransTypeManager &operator=(DTransTypeManager &&) = delete;

  // Return associated LLVMContext.
  LLVMContext &getContext() const { return Ctx; }

  // Create a DTransAtomicType to represent a void type or first class llvm
  // type. Returns existing type, if one already exists.
  DTransAtomicType *getOrCreateAtomicType(llvm::Type *Ty);

  // Get or create a DTransPointerType to represent a pointer to some type.
  // Returns existing type, if one already exists.
  DTransPointerType *getOrCreatePointerType(DTransType *PointeeTy);

  // Get Create a DTransStructType to represent a named structure type. Returns
  // existing type, if one already exists.
  DTransStructType *getOrCreateStructType(llvm::StructType *Ty);

  // Get an existing DTransStructType, if one exists with the given \p Name.
  DTransStructType *getStructType(StringRef Name) const;

  // Create a DTransStructType for a literal type, populating the field types
  // with the types in \p FieldTypes. Returns existing type, if one already
  // exists.
  DTransStructType *
  getOrCreateLiteralStructType(LLVMContext &Ctx,
                               ArrayRef<DTransType *> FieldTypes);

  // Create a DTransArrayType for [\p Num x \p ElemType]. Returns existing type,
  // if one already exists.
  DTransArrayType *getOrCreateArrayType(DTransType *ElemType, uint64_t Num);

  // Create a DTransVectorType for <\p Num x \p ElemType>. Returns existing
  // type, if one already exists.
  DTransVectorType *getOrCreateVectorType(DTransType *ElemType, uint64_t Num);

  // Create a DTransFunctionType with a signature based on the \p DTRetTy and \p
  // ParamTypes. Returns existing type, if one already exists.
  DTransFunctionType *
  getOrCreateFunctionType(DTransType *DTRetTy,
                          ArrayRef<DTransType *> ParamTypes,
                          bool IsVarArg);

  // We don't supply a method for looking up a type based on an
  // llvm::FunctionType to get a DTransFunctionType because this cannot be done
  // unambiguously. For example, the type "void (ptr, i32)" could be used for
  // "void (i32*, i32)" or "void (%struct.ty*, i32)"
  DTransFunctionType *createFunctionType(size_t NumArgs, bool IsVarArg);

  // Return 'true' if it is possible to directly convert the llvm::type into a
  // DTransType.
  //
  // A simple type is one that is not a pointer, and does not contain any
  // elements that are pointers. An exception is made for the case of named
  // structures, as long as a type for the structure has already been created
  // based on decoding DTrans metadata.
  bool isSimpleType(llvm::Type *Ty) const;

  // Create a DTransType for the llvm::type. The input type must pass the test
  // of isSimpleType to create a DTransType, otherwise a nullptr will be
  // returned.
  DTransType *getOrCreateSimpleType(llvm::Type *Ty);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printTypes() const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // Return the DTransType for \p Ty, if one exists. Otherwise, returns nullptr.
  // This is only useful for first class llvm types and non-literal structure
  // types, because those are the only types that will have a one-to-one mapping
  // between llvm::Type objects and llvm::DTransType objects.
  DTransType *findType(llvm::Type *Ty) const;

  // Return a vector of all the named structures. i.e. No literal structure
  // types will be included in the vector returned.
  std::vector<DTransStructType *> getIdentifiedStructTypes() const;

  // This type will store pointers to all the DTransType objects created, so
  // that all of them can be easily visited using the dtrans_types() method.
  using DTransTypesVector = std::vector<DTransType*>;

  // Iterator for DTransTypesVector
  struct dtrans_types_iterator
    : public iterator_adaptor_base<
    dtrans_types_iterator, DTransTypesVector::iterator,
    std::forward_iterator_tag, DTransTypesVector::size_type> {
    explicit dtrans_types_iterator(DTransTypesVector::iterator X)
      : iterator_adaptor_base(X) {}

    DTransTypesVector::value_type operator*() const { return *I; }
    DTransTypesVector::value_type operator->() const { return operator*(); }
  };

  iterator_range<dtrans_types_iterator> dtrans_types() {
    return make_range(dtrans_types_iterator(AllDTransTypes.begin()),
      dtrans_types_iterator(AllDTransTypes.end()));
  }

private:
  void DeleteType(DTransType *DTTy);

  LLVMContext &Ctx;

  // Create a type to a type that does not reference any pointers. If there are
  // pointer references returns nullptr. i.e. i64, %struct.t1, [8 x i32] will
  // return a corresponding DTransType. ptr*, [8 x ptr] will return nullptr.
  // This is used for generating field members for structure representations.
  DTransType *getFieldMemberType(llvm::Type *Ty);

  // Mapping for primitive/non-literal structure types to DTransType object.
  DenseMap<llvm::Type *, DTransType *> TypeInfoMap;

  // Mapping of named structures to DTransStructType objects.
  StringMap<DTransStructType *> StructTypeInfoMap;

  // Mapping from a DTransType to a DTransPointerType that represents a pointer
  // to the type
  DenseMap<DTransType *, DTransPointerType *> PointerTypeInfoMap;

  // Mapping from a {DTransType, Num} pair to a DTransArrayType to represents of
  // array of Num elememts of DTransType objects
  DenseMap<std::pair<DTransType *, uint64_t>, DTransArrayType *>
      ArrayTypeInfoMap;

  // Mapping from a {DTransType, Num} pair to a DTransArrayType to represents of
  // vector of Num elememts of DTransType objects
  DenseMap<std::pair<DTransType *, uint64_t>, DTransVectorType *>
      VecTypeInfoMap;

  // List of all literal struct types allocation that need to be destroyed.
  SmallVector<DTransStructType *, 32> LitStructTypeVec;

  // This allocator will be used for all the DTransFunctionTypeNode objects so
  // that the entire memory pool can be released, instead of deleting each
  // object, since there is no need to run destructors on the
  // DTransFunctionTypeNode objects.
  BumpPtrAllocator Allocator;

  // Set of handles to unique DTransFunctionType objects allocated. The
  // DTransFunctionType objects referenced by the DTransFunctionTypeNode need to
  // be deallocated prior to the destruction of this set.
  FoldingSet<DTransFunctionTypeNode> FunctionTypeNodes;

  // All the DTransType objects created.
  // This contains the same pointers that are stored in the categorized
  // maps/sets that are used when searching whether a specific type has been
  // created yet or not: TypeInfoMap, StructTypeInfoMap, PointerTypeInfoMap,
  // ArrayTypeInfoMap, VecTypeInfoMap, FunctionTypeNodes.
  DTransTypesVector AllDTransTypes;
};

// This class provides a simple interface for passes that need to create DTrans
// type metadata.
class DTransTypeBuilder {
public:
  DTransTypeBuilder(DTransTypeManager &TM) : TM(TM) {}

  DTransType *getVoidTy();
  DTransType *getIntNTy(unsigned N);
  DTransPointerType *getPointerToTy(DTransType *DTy);
  DTransStructType *getStructTy(StructType *Ty);
  DTransFunctionType *getFunctionType(DTransType *DTRetTy,
                                      ArrayRef<DTransType *> ParamTypes,
                                      bool IsVarArg);

  void populateDTransStructType(DTransStructType *DStructTy,
                                ArrayRef<DTransType *> FieldTypes);

private:
  DTransTypeManager &TM;
};

} // namespace dtransOP
} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSTYPES_H
