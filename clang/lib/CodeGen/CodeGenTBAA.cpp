//===--- CodeGenTypes.cpp - TBAA information for LLVM CodeGen -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is the code that manages TBAA information and defines the TBAA policy
// for the optimizer to use. Relevant standards text includes:
//
//   C99 6.5p7
//   C++ [basic.lval] (p10 in n3126, p15 in some earlier versions)
//
//===----------------------------------------------------------------------===//

#include "CodeGenTBAA.h"
#if INTEL_CUSTOMIZATION
#include "CGRecordLayout.h"
#include "CodeGenModule.h"
#endif // INTEL_CUSTOMIZATION
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Mangle.h"
#include "clang/AST/RecordLayout.h"
#include "clang/Frontend/CodeGenOptions.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
using namespace clang;
using namespace CodeGen;

#if INTEL_CUSTOMIZATION
// CQ#379144 TBAA for pointers and arrays.
bool CodeGenTBAA::canCreateUniqueTBAA(const Type *Ty) {
  if (isa<BuiltinType>(Ty))
    return true;
  if (const PointerType *PTy = dyn_cast<PointerType>(Ty))
    return canCreateUniqueTBAA(Context.getCanonicalType(
        PTy->getPointeeType().getTypePtr()));
  if (const ConstantArrayType *ArrayTy = dyn_cast<ConstantArrayType>(Ty))
    return canCreateUniqueTBAA(Context.getCanonicalType(
        ArrayTy->getElementType().getTypePtr()));
  if (const FunctionProtoType *FnTy = dyn_cast<FunctionProtoType>(Ty)) {
    if (!canCreateUniqueTBAA(Context.getCanonicalType(
          FnTy->getReturnType().getTypePtr())))
      return false;
    for (unsigned i = 0, n = FnTy->getNumParams(); i < n; ++i)
      if (!canCreateUniqueTBAA(Context.getCanonicalType(
            FnTy->getParamType(i).getTypePtr())))
        return false;
    return true;
  }
  if (const EnumType *EnumTy = dyn_cast<EnumType>(Ty))
    return Features.CPlusPlus && EnumTy->getDecl()->isExternallyVisible();
  // Remove this for now as it creates bad TBAA (cq#416741).
  // if (const RecordType *RecordTy = dyn_cast<RecordType>(Ty))
  //   return RecordTy->getDecl()->isExternallyVisible();
  return false;
}

llvm::MDNode *CodeGenTBAA::createTBAAPointerType(const PointerType *PTy) {
  if (!canCreateUniqueTBAA(PTy))
    return createScalarTypeNode("unspecified pointer", getChar(), /*Size=*/1);

  SmallString<256> OutName;
  llvm::raw_svector_ostream Out(OutName);
  Out << "pointer@";
  MContext.mangleTypeName(QualType(PTy, 0), Out);
  return createScalarTypeNode(OutName, getChar(), /*Size=*/1);
}
#endif // INTEL_CUSTOMIZATION

CodeGenTBAA::CodeGenTBAA(ASTContext &Ctx, llvm::Module &M,
                         const CodeGenOptions &CGO,
                         const LangOptions &Features, MangleContext &MContext)
  : Context(Ctx), Module(M), CodeGenOpts(CGO),
    Features(Features), MContext(MContext), MDHelper(M.getContext()),
    Root(nullptr), Char(nullptr)
{}

CodeGenTBAA::~CodeGenTBAA() {
}

llvm::MDNode *CodeGenTBAA::getRoot() {
  // Define the root of the tree. This identifies the tree, so that
  // if our LLVM IR is linked with LLVM IR from a different front-end
  // (or a different version of this front-end), their TBAA trees will
  // remain distinct, and the optimizer will treat them conservatively.
  if (!Root) {
    if (Features.CPlusPlus)
      Root = MDHelper.createTBAARoot("Simple C++ TBAA");
    else
      Root = MDHelper.createTBAARoot("Simple C/C++ TBAA");
  }

  return Root;
}

llvm::MDNode *CodeGenTBAA::createScalarTypeNode(StringRef Name,
                                                llvm::MDNode *Parent,
                                                uint64_t Size) {
  if (CodeGenOpts.NewStructPathTBAA) {
    llvm::Metadata *Id = MDHelper.createString(Name);
    return MDHelper.createTBAATypeNode(Parent, Size, Id);
  }
  return MDHelper.createTBAAScalarTypeNode(Name, Parent);
}

llvm::MDNode *CodeGenTBAA::getChar() {
  // Define the root of the tree for user-accessible memory. C and C++
  // give special powers to char and certain similar types. However,
  // these special powers only cover user-accessible memory, and doesn't
  // include things like vtables.
  if (!Char)
    Char = createScalarTypeNode("omnipotent char", getRoot(), /* Size= */ 1);

  return Char;
}

static bool TypeHasMayAlias(QualType QTy) {
  // Tagged types have declarations, and therefore may have attributes.
  if (const TagType *TTy = dyn_cast<TagType>(QTy))
    return TTy->getDecl()->hasAttr<MayAliasAttr>();

  // Typedef types have declarations, and therefore may have attributes.
  if (const TypedefType *TTy = dyn_cast<TypedefType>(QTy)) {
    if (TTy->getDecl()->hasAttr<MayAliasAttr>())
      return true;
    // Also, their underlying types may have relevant attributes.
    return TypeHasMayAlias(TTy->desugar());
  }

  return false;
}

/// Check if the given type is a valid base type to be used in access tags.
static bool isValidBaseType(QualType QTy) {
  if (QTy->isReferenceType())
    return false;
  if (const RecordType *TTy = QTy->getAs<RecordType>()) {
    const RecordDecl *RD = TTy->getDecl()->getDefinition();
    // Incomplete types are not valid base access types.
    if (!RD)
      return false;
    if (RD->hasFlexibleArrayMember())
      return false;
    // RD can be struct, union, class, interface or enum.
    // For now, we only handle struct and class.
    if (RD->isStruct() || RD->isClass())
      return true;
  }
  return false;
}

llvm::MDNode *CodeGenTBAA::getTypeInfoHelper(const Type *Ty) {
  uint64_t Size = Context.getTypeSizeInChars(Ty).getQuantity();

  // Handle builtin types.
  if (const BuiltinType *BTy = dyn_cast<BuiltinType>(Ty)) {
    switch (BTy->getKind()) {
    // Character types are special and can alias anything.
    // In C++, this technically only includes "char" and "unsigned char",
    // and not "signed char". In C, it includes all three. For now,
    // the risk of exploiting this detail in C++ seems likely to outweigh
    // the benefit.
    case BuiltinType::Char_U:
    case BuiltinType::Char_S:
    case BuiltinType::UChar:
    case BuiltinType::SChar:
      return getChar();

    // Unsigned types can alias their corresponding signed types.
    case BuiltinType::UShort:
      return getTypeInfo(Context.ShortTy);
    case BuiltinType::UInt:
      return getTypeInfo(Context.IntTy);
    case BuiltinType::ULong:
      return getTypeInfo(Context.LongTy);
    case BuiltinType::ULongLong:
      return getTypeInfo(Context.LongLongTy);
    case BuiltinType::UInt128:
      return getTypeInfo(Context.Int128Ty);

    // Treat all other builtin types as distinct types. This includes
    // treating wchar_t, char16_t, and char32_t as distinct from their
    // "underlying types".
    default:
      return createScalarTypeNode(BTy->getName(Features), getChar(), Size);
    }
  }

  // C++1z [basic.lval]p10: "If a program attempts to access the stored value of
  // an object through a glvalue of other than one of the following types the
  // behavior is undefined: [...] a char, unsigned char, or std::byte type."
  if (Ty->isStdByteType())
    return getChar();

  // Handle pointers and references.
#if INTEL_CUSTOMIZATION
  // CQ#379144 TBAA for pointers.
  if (Features.IntelCompat) {
    if (const PointerType *PTy = dyn_cast<PointerType>(Ty))
      return MetadataCache[Ty] = createTBAAPointerType(PTy);
  }
#endif // INTEL_CUSTOMIZATION
  // TODO: Implement C++'s type "similarity" and consider dis-"similar"
  // pointers distinct.
  if (Ty->isPointerType() || Ty->isReferenceType())
    return createScalarTypeNode("any pointer", getChar(), Size);

  // Accesses to arrays are accesses to objects of their element types.
  if (CodeGenOpts.NewStructPathTBAA && Ty->isArrayType())
    return getTypeInfo(cast<ArrayType>(Ty)->getElementType());

  // Enum types are distinct types. In C++ they have "underlying types",
  // however they aren't related for TBAA.
  if (const EnumType *ETy = dyn_cast<EnumType>(Ty)) {
    // In C++ mode, types have linkage, so we can rely on the ODR and
    // on their mangled names, if they're external.
    // TODO: Is there a way to get a program-wide unique name for a
    // decl with local linkage or no linkage?
    if (!Features.CPlusPlus || !ETy->getDecl()->isExternallyVisible())
      return getChar();

    SmallString<256> OutName;
    llvm::raw_svector_ostream Out(OutName);
    MContext.mangleTypeName(QualType(ETy, 0), Out);
    return createScalarTypeNode(OutName, getChar(), Size);
  }
#if INTEL_CUSTOMIZATION
  // CQ#379144 TBAA for arrays.
  if (Features.IntelCompat) {
    if (const ConstantArrayType* CATy = dyn_cast<ConstantArrayType>(Ty)) {
      if (canCreateUniqueTBAA(Ty)) {
        SmallString<256> OutName;
        llvm::raw_svector_ostream Out(OutName);
        Out << "array@";
        MContext.mangleTypeName(QualType(Ty, 0), Out);
        llvm::MDNode *Parent = getTypeInfo(CATy->getElementType());
        uint64_t Size = Context.getTypeSizeInChars(Ty).getQuantity();
        return MetadataCache[Ty] = createScalarTypeNode(OutName, Parent, Size);
      }
    }
  }
#endif // INTEL_CUSTOMIZATION

  // For now, handle any other kind of type conservatively.
  return getChar();
}

llvm::MDNode *CodeGenTBAA::getTypeInfo(QualType QTy) {
  // At -O0 or relaxed aliasing, TBAA is not emitted for regular types.
  if (CodeGenOpts.OptimizationLevel == 0 || CodeGenOpts.RelaxedAliasing)
    return nullptr;

  // If the type has the may_alias attribute (even on a typedef), it is
  // effectively in the general char alias class.
  if (TypeHasMayAlias(QTy))
    return getChar();

  // We need this function to not fall back to returning the "omnipotent char"
  // type node for aggregate and union types. Otherwise, any dereference of an
  // aggregate will result into the may-alias access descriptor, meaning all
  // subsequent accesses to direct and indirect members of that aggregate will
  // be considered may-alias too.
  // TODO: Combine getTypeInfo() and getBaseTypeInfo() into a single function.
  if (isValidBaseType(QTy))
    return getBaseTypeInfo(QTy);

  const Type *Ty = Context.getCanonicalType(QTy).getTypePtr();
  if (llvm::MDNode *N = MetadataCache[Ty])
    return N;

  // Note that the following helper call is allowed to add new nodes to the
  // cache, which invalidates all its previously obtained iterators. So we
  // first generate the node for the type and then add that node to the cache.
  llvm::MDNode *TypeNode = getTypeInfoHelper(Ty);
  return MetadataCache[Ty] = TypeNode;
}

TBAAAccessInfo CodeGenTBAA::getAccessInfo(QualType AccessType) {
  // Pointee values may have incomplete types, but they shall never be
  // dereferenced.
#if INTEL_CUSTOMIZATION
  if (AccessType->isIncompleteType()) {
    if (Features.IntelCompat && AccessType->isIncompleteArrayType()) {
      if (!AccessType->getArrayElementTypeNoTypeQual()->isIncompleteType())
        return TBAAAccessInfo(getTypeInfo(AccessType), 0);
    }
    return TBAAAccessInfo::getIncompleteInfo();
  }
#endif // INTEL_CUSTOMIZATION

  if (TypeHasMayAlias(AccessType))
    return TBAAAccessInfo::getMayAliasInfo();

  uint64_t Size = Context.getTypeSizeInChars(AccessType).getQuantity();
  return TBAAAccessInfo(getTypeInfo(AccessType), Size);
}

TBAAAccessInfo CodeGenTBAA::getVTablePtrAccessInfo(llvm::Type *VTablePtrType) {
  llvm::DataLayout DL(&Module);
  unsigned Size = DL.getPointerTypeSize(VTablePtrType);
  return TBAAAccessInfo(createScalarTypeNode("vtable pointer", getRoot(), Size),
                        Size);
}

bool
CodeGenTBAA::CollectFields(uint64_t BaseOffset,
                           QualType QTy,
                           SmallVectorImpl<llvm::MDBuilder::TBAAStructField> &
                             Fields,
                           bool MayAlias) {
  /* Things not handled yet include: C++ base classes, bitfields, */

  if (const RecordType *TTy = QTy->getAs<RecordType>()) {
    const RecordDecl *RD = TTy->getDecl()->getDefinition();
    if (RD->hasFlexibleArrayMember())
      return false;

    // TODO: Handle C++ base classes.
    if (const CXXRecordDecl *Decl = dyn_cast<CXXRecordDecl>(RD))
      if (Decl->bases_begin() != Decl->bases_end())
        return false;

    const ASTRecordLayout &Layout = Context.getASTRecordLayout(RD);

    unsigned idx = 0;
    for (RecordDecl::field_iterator i = RD->field_begin(),
         e = RD->field_end(); i != e; ++i, ++idx) {
      uint64_t Offset = BaseOffset +
                        Layout.getFieldOffset(idx) / Context.getCharWidth();
      QualType FieldQTy = i->getType();
      if (!CollectFields(Offset, FieldQTy, Fields,
                         MayAlias || TypeHasMayAlias(FieldQTy)))
        return false;
    }
    return true;
  }

  /* Otherwise, treat whatever it is as a field. */
  uint64_t Offset = BaseOffset;
  uint64_t Size = Context.getTypeSizeInChars(QTy).getQuantity();
  llvm::MDNode *TBAAType = MayAlias ? getChar() : getTypeInfo(QTy);
  llvm::MDNode *TBAATag = getAccessTagInfo(TBAAAccessInfo(TBAAType, Size));
  Fields.push_back(llvm::MDBuilder::TBAAStructField(Offset, Size, TBAATag));
  return true;
}

llvm::MDNode *
CodeGenTBAA::getTBAAStructInfo(QualType QTy) {
  const Type *Ty = Context.getCanonicalType(QTy).getTypePtr();

  if (llvm::MDNode *N = StructMetadataCache[Ty])
    return N;

  SmallVector<llvm::MDBuilder::TBAAStructField, 4> Fields;
  if (CollectFields(0, QTy, Fields, TypeHasMayAlias(QTy)))
    return MDHelper.createTBAAStructNode(Fields);

  // For now, handle any other kind of type conservatively.
  return StructMetadataCache[Ty] = nullptr;
}

llvm::MDNode *CodeGenTBAA::getBaseTypeInfoHelper(const Type *Ty) {
  if (auto *TTy = dyn_cast<RecordType>(Ty)) {
    assert(CGM && "Module is null!"); // INTEL
    const RecordDecl *RD = TTy->getDecl()->getDefinition();
    const CGRecordLayout &RL = CGM->getTypes().getCGRecordLayout(RD); // INTEL
    const ASTRecordLayout &Layout = Context.getASTRecordLayout(RD);
    SmallVector<llvm::MDBuilder::TBAAStructField, 4> Fields;
    for (FieldDecl *Field : RD->fields()) {
      QualType FieldQTy = Field->getType();
#if INTEL_CUSTOMIZATION
      if (CGM->getLangOpts().isIntelCompat(LangOptions::IntelTBAABF) &&
          Field->isBitField() && Field->getBitWidthValue(Context) > 0) {
        const CGBitFieldInfo &Info = RL.getBitFieldInfo(Field);
        FieldQTy = Context.getIntTypeForBitwidth(Info.StorageSize, 0);
        if (FieldQTy.isNull())
          /* If the Info.StorageSize is irregular, say 48, then there
             is no corresponding int type: return nullptr */
          /* Andrei suggested, "Depending on the guarantees language provides
             we might want to *always* create special types for the bitfields,
             something like:
             !bit_field_type_of_N_bytes =
                 !{!"bitfield_type_N", !"omnipotent char", i64 0}
           */
          return BaseTypeMetadataCache[Ty] = nullptr;
      } else {
        FieldQTy = Field->getType();
      }
#endif // INTEL_CUSTOMIZATION
      llvm::MDNode *TypeNode = isValidBaseType(FieldQTy) ?
          getBaseTypeInfo(FieldQTy) : getTypeInfo(FieldQTy);
      if (!TypeNode)
        return BaseTypeMetadataCache[Ty] = nullptr;

      uint64_t BitOffset = Layout.getFieldOffset(Field->getFieldIndex());
      uint64_t Offset = Context.toCharUnitsFromBits(BitOffset).getQuantity();
      uint64_t Size = Context.getTypeSizeInChars(FieldQTy).getQuantity();
      Fields.push_back(llvm::MDBuilder::TBAAStructField(Offset, Size,
                                                        TypeNode));
    }

    SmallString<256> OutName;
#if INTEL_CUSTOMIZATION
    // CQ#379144 Intel TBAA.
    llvm::raw_svector_ostream Out(OutName);
    if (Features.IntelCompat)
      Out << "struct@";
    if (Features.CPlusPlus) {
      // Don't use the mangler for C code.
      MContext.mangleTypeName(QualType(Ty, 0), Out);
    } else {
      Out << RD->getName();
    }
#endif // INTEL_CUSTOMIZATION

    if (CodeGenOpts.NewStructPathTBAA) {
      llvm::MDNode *Parent = getChar();
      uint64_t Size = Context.getTypeSizeInChars(Ty).getQuantity();
      llvm::Metadata *Id = MDHelper.createString(OutName);
      return MDHelper.createTBAATypeNode(Parent, Size, Id, Fields);
    }

    // Create the struct type node with a vector of pairs (offset, type).
    SmallVector<std::pair<llvm::MDNode*, uint64_t>, 4> OffsetsAndTypes;
    for (const auto &Field : Fields)
        OffsetsAndTypes.push_back(std::make_pair(Field.Type, Field.Offset));
    return MDHelper.createTBAAStructTypeNode(OutName, OffsetsAndTypes);
  }

  return nullptr;
}

llvm::MDNode *CodeGenTBAA::getBaseTypeInfo(QualType QTy) {
  if (!isValidBaseType(QTy))
    return nullptr;

  const Type *Ty = Context.getCanonicalType(QTy).getTypePtr();
  if (llvm::MDNode *N = BaseTypeMetadataCache[Ty])
    return N;

  // Note that the following helper call is allowed to add new nodes to the
  // cache, which invalidates all its previously obtained iterators. So we
  // first generate the node for the type and then add that node to the cache.
  llvm::MDNode *TypeNode = getBaseTypeInfoHelper(Ty);
  return BaseTypeMetadataCache[Ty] = TypeNode;
}

llvm::MDNode *CodeGenTBAA::getAccessTagInfo(TBAAAccessInfo Info) {
  assert(!Info.isIncomplete() && "Access to an object of an incomplete type!");

  if (Info.isMayAlias())
    Info = TBAAAccessInfo(getChar(), Info.Size);

  if (!Info.AccessType)
    return nullptr;

  if (!CodeGenOpts.StructPathTBAA)
    Info = TBAAAccessInfo(Info.AccessType, Info.Size);

  llvm::MDNode *&N = AccessTagMetadataCache[Info];
  if (N)
    return N;

  if (!Info.BaseType) {
    Info.BaseType = Info.AccessType;
    assert(!Info.Offset && "Nonzero offset for an access with no base type!");
  }
  if (CodeGenOpts.NewStructPathTBAA) {
    return N = MDHelper.createTBAAAccessTag(Info.BaseType, Info.AccessType,
                                            Info.Offset, Info.Size);
  }
  return N = MDHelper.createTBAAStructTagNode(Info.BaseType, Info.AccessType,
                                              Info.Offset);
}

TBAAAccessInfo CodeGenTBAA::mergeTBAAInfoForCast(TBAAAccessInfo SourceInfo,
                                                 TBAAAccessInfo TargetInfo) {
  if (SourceInfo.isMayAlias() || TargetInfo.isMayAlias())
    return TBAAAccessInfo::getMayAliasInfo();
  return TargetInfo;
}

TBAAAccessInfo
CodeGenTBAA::mergeTBAAInfoForConditionalOperator(TBAAAccessInfo InfoA,
                                                 TBAAAccessInfo InfoB) {
  if (InfoA == InfoB)
    return InfoA;

  if (!InfoA || !InfoB)
    return TBAAAccessInfo();

  if (InfoA.isMayAlias() || InfoB.isMayAlias())
    return TBAAAccessInfo::getMayAliasInfo();

  // TODO: Implement the rest of the logic here. For example, two accesses
  // with same final access types result in an access to an object of that final
  // access type regardless of their base types.
  return TBAAAccessInfo::getMayAliasInfo();
}

TBAAAccessInfo
CodeGenTBAA::mergeTBAAInfoForMemoryTransfer(TBAAAccessInfo DestInfo,
                                            TBAAAccessInfo SrcInfo) {
  if (DestInfo == SrcInfo)
    return DestInfo;

  if (!DestInfo || !SrcInfo)
    return TBAAAccessInfo();

  if (DestInfo.isMayAlias() || SrcInfo.isMayAlias())
    return TBAAAccessInfo::getMayAliasInfo();

  // TODO: Implement the rest of the logic here. For example, two accesses
  // with same final access types result in an access to an object of that final
  // access type regardless of their base types.
  return TBAAAccessInfo::getMayAliasInfo();
}
