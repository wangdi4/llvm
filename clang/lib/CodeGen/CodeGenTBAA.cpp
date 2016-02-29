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
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Mangle.h"
#include "clang/AST/RecordLayout.h"
#include "clang/Frontend/CodeGenOptions.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Type.h"
using namespace clang;
using namespace CodeGen;

CodeGenTBAA::CodeGenTBAA(ASTContext &Ctx, llvm::LLVMContext& VMContext,
                         const CodeGenOptions &CGO,
                         const LangOptions &Features, MangleContext &MContext)
  : Context(Ctx), CodeGenOpts(CGO), Features(Features), MContext(MContext),
    MDHelper(VMContext), Root(nullptr), Char(nullptr) {
}

CodeGenTBAA::~CodeGenTBAA() {
}

llvm::MDNode *CodeGenTBAA::getRoot() {
  // Define the root of the tree. This identifies the tree, so that
  // if our LLVM IR is linked with LLVM IR from a different front-end
  // (or a different version of this front-end), their TBAA trees will
  // remain distinct, and the optimizer will treat them conservatively.
  if (!Root)
    Root = MDHelper.createTBAARoot("Simple C/C++ TBAA");

  return Root;
}

// For both scalar TBAA and struct-path aware TBAA, the scalar type has the
// same format: name, parent node, and offset.
llvm::MDNode *CodeGenTBAA::createTBAAScalarType(StringRef Name,
                                                llvm::MDNode *Parent) {
  return MDHelper.createTBAAScalarTypeNode(Name, Parent);
}

llvm::MDNode *CodeGenTBAA::getChar() {
  // Define the root of the tree for user-accessible memory. C and C++
  // give special powers to char and certain similar types. However,
  // these special powers only cover user-accessible memory, and doesn't
  // include things like vtables.
  if (!Char)
    Char = createTBAAScalarType("omnipotent char", getRoot());

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

llvm::MDNode *
CodeGenTBAA::getTBAAInfo(QualType QTy) {
  // At -O0 or relaxed aliasing, TBAA is not emitted for regular types.
  if (CodeGenOpts.OptimizationLevel == 0 || CodeGenOpts.RelaxedAliasing)
    return nullptr;

  // If the type has the may_alias attribute (even on a typedef), it is
  // effectively in the general char alias class.
  if (TypeHasMayAlias(QTy))
    return getChar();

  const Type *Ty = Context.getCanonicalType(QTy).getTypePtr();

  if (llvm::MDNode *N = MetadataCache[Ty])
    return N;

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
      return getTBAAInfo(Context.ShortTy);
    case BuiltinType::UInt:
      return getTBAAInfo(Context.IntTy);
    case BuiltinType::ULong:
      return getTBAAInfo(Context.LongTy);
    case BuiltinType::ULongLong:
      return getTBAAInfo(Context.LongLongTy);
    case BuiltinType::UInt128:
      return getTBAAInfo(Context.Int128Ty);

    // Treat all other builtin types as distinct types. This includes
    // treating wchar_t, char16_t, and char32_t as distinct from their
    // "underlying types".
    default:
      return MetadataCache[Ty] =
        createTBAAScalarType(BTy->getName(Features), getChar());
    }
  }

  // Handle pointers.
#if INTEL_CUSTOMIZATION
  // CQ#379144 TBAA for pointers.
  if (Features.IntelCompat) {
    if (const PointerType *PTy = dyn_cast<PointerType>(Ty))
      return MetadataCache[Ty] = createTBAAPointerType(PTy);
  } else
#endif // INTEL_CUSTOMIZATION
  // TODO: Implement C++'s type "similarity" and consider dis-"similar"
  // pointers distinct.
  if (Ty->isPointerType())
    return MetadataCache[Ty] = createTBAAScalarType("any pointer",
                                                    getChar());

  // Enum types are distinct types. In C++ they have "underlying types",
  // however they aren't related for TBAA.
  if (const EnumType *ETy = dyn_cast<EnumType>(Ty)) {
    // In C++ mode, types have linkage, so we can rely on the ODR and
    // on their mangled names, if they're external.
    // TODO: Is there a way to get a program-wide unique name for a
    // decl with local linkage or no linkage?
    if (!Features.CPlusPlus || !ETy->getDecl()->isExternallyVisible())
      return MetadataCache[Ty] = getChar();

    SmallString<256> OutName;
    llvm::raw_svector_ostream Out(OutName);
    MContext.mangleTypeName(QualType(ETy, 0), Out);
    return MetadataCache[Ty] = createTBAAScalarType(OutName, getChar());
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
        llvm::MDNode *Parent = getTBAAInfo(CATy->getElementType());
        return MetadataCache[Ty] = createTBAAScalarType(OutName, Parent);
      }
    }
  }
#endif // INTEL_CUSTOMIZATION

  // For now, handle any other kind of type conservatively.
  return MetadataCache[Ty] = getChar();
}

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
  if (const RecordType *RecordTy = dyn_cast<RecordType>(Ty))
    return RecordTy->getDecl()->isExternallyVisible();
  return false;
}

llvm::MDNode *CodeGenTBAA::createTBAAPointerType(const PointerType *PTy) {
  if (!canCreateUniqueTBAA(PTy))
    return createTBAAScalarType("unspecified pointer", getChar());

  SmallString<256> OutName;
  llvm::raw_svector_ostream Out(OutName);
  Out << "pointer@";
  MContext.mangleTypeName(QualType(PTy, 0), Out);
  return createTBAAScalarType(OutName, getChar());
}
#endif // INTEL_CUSTOMIZATION
llvm::MDNode *CodeGenTBAA::getTBAAInfoForVTablePtr() {
  return createTBAAScalarType("vtable pointer", getRoot());
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
  llvm::MDNode *TBAAInfo = MayAlias ? getChar() : getTBAAInfo(QTy);
  llvm::MDNode *TBAATag = getTBAAScalarTagInfo(TBAAInfo);
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

/// Check if the given type can be handled by path-aware TBAA.
static bool isTBAAPathStruct(QualType QTy) {
  if (const RecordType *TTy = QTy->getAs<RecordType>()) {
    const RecordDecl *RD = TTy->getDecl()->getDefinition();
    if (RD->hasFlexibleArrayMember())
      return false;
    // RD can be struct, union, class, interface or enum.
    // For now, we only handle struct and class.
    if (RD->isStruct() || RD->isClass())
      return true;
  }
  return false;
}

llvm::MDNode *
CodeGenTBAA::getTBAAStructTypeInfo(QualType QTy) {
  const Type *Ty = Context.getCanonicalType(QTy).getTypePtr();
  assert(isTBAAPathStruct(QTy));

  if (llvm::MDNode *N = StructTypeMetadataCache[Ty])
    return N;

  if (const RecordType *TTy = QTy->getAs<RecordType>()) {
    const RecordDecl *RD = TTy->getDecl()->getDefinition();

    const ASTRecordLayout &Layout = Context.getASTRecordLayout(RD);
    SmallVector <std::pair<llvm::MDNode*, uint64_t>, 4> Fields;
    unsigned idx = 0;
    for (RecordDecl::field_iterator i = RD->field_begin(),
         e = RD->field_end(); i != e; ++i, ++idx) {
      QualType FieldQTy = i->getType();
      llvm::MDNode *FieldNode;
      if (isTBAAPathStruct(FieldQTy))
        FieldNode = getTBAAStructTypeInfo(FieldQTy);
      else
        FieldNode = getTBAAInfo(FieldQTy);
      if (!FieldNode)
        return StructTypeMetadataCache[Ty] = nullptr;
      Fields.push_back(std::make_pair(
          FieldNode, Layout.getFieldOffset(idx) / Context.getCharWidth()));
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
    // Create the struct type node with a vector of pairs (offset, type).
    return StructTypeMetadataCache[Ty] =
      MDHelper.createTBAAStructTypeNode(OutName, Fields);
  }

  return StructMetadataCache[Ty] = nullptr;
}

/// Return a TBAA tag node for both scalar TBAA and struct-path aware TBAA.
llvm::MDNode *
CodeGenTBAA::getTBAAStructTagInfo(QualType BaseQTy, llvm::MDNode *AccessNode,
                                  uint64_t Offset) {
  if (!AccessNode)
    return nullptr;

  if (!CodeGenOpts.StructPathTBAA)
    return getTBAAScalarTagInfo(AccessNode);

  const Type *BTy = Context.getCanonicalType(BaseQTy).getTypePtr();
  TBAAPathTag PathTag = TBAAPathTag(BTy, AccessNode, Offset);
  if (llvm::MDNode *N = StructTagMetadataCache[PathTag])
    return N;

  llvm::MDNode *BNode = nullptr;
  if (isTBAAPathStruct(BaseQTy))
    BNode  = getTBAAStructTypeInfo(BaseQTy);
#if INTEL_CUSTOMIZATION
  // CQ#379144 TBAA for arrays.
  else if (BTy->isConstantArrayType() && canCreateUniqueTBAA(BTy))
    BNode = getTBAAInfo(BaseQTy);
  if (!BNode) {
    // It is same as scalar so also remember it as a scalar.
    return StructTagMetadataCache[PathTag] = getTBAAScalarTagInfo(AccessNode);
  }
#endif // INTEL_CUSTOMIZATION

  return StructTagMetadataCache[PathTag] =
    MDHelper.createTBAAStructTagNode(BNode, AccessNode, Offset);
}

llvm::MDNode *
CodeGenTBAA::getTBAAScalarTagInfo(llvm::MDNode *AccessNode) {
  if (!AccessNode)
    return nullptr;
  if (llvm::MDNode *N = ScalarTagMetadataCache[AccessNode])
    return N;

  return ScalarTagMetadataCache[AccessNode] =
    MDHelper.createTBAAStructTagNode(AccessNode, AccessNode, 0);
}
