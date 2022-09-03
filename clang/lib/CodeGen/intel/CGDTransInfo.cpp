#if INTEL_FEATURE_SW_DTRANS
//==--- CGDTransInfo.cpp - DTrans Type Info Codegen ------------*- C++ -*---==//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "CodeGenModule.h"
#include "CGRecordLayout.h"
#include "llvm/IR/TypeFinder.h"

using namespace clang;
using namespace CodeGen;

namespace {
class StructEmitTy {
  using DataTy = llvm::PointerIntPair<const RecordDecl *, 1>;
  DataTy Data;

  StructEmitTy(DataTy D) : Data(D) {}

public:
  enum IsBase { Standard = 0, Base = 1 };
  StructEmitTy(const RecordDecl *RD, IsBase B) : Data(RD, B) {
    assert(RD && "Cannot be created without a valid RecordDecl");
  }

  const RecordDecl *getRecordDecl() { return Data.getPointer(); }
  IsBase isBase() { return Data.getInt() ? Base : Standard; }

  unsigned getHashValue() {
    return llvm::DenseMapInfo<DataTy>::getHashValue(Data);
  }

  static StructEmitTy getTombstoneKey() {
    return StructEmitTy(llvm::DenseMapInfo<DataTy>::getTombstoneKey());
  }
  static StructEmitTy getEmptyKey() {
    return StructEmitTy(llvm::DenseMapInfo<DataTy>::getEmptyKey());
  }
  bool operator==(const StructEmitTy &RHS) const { return Data == RHS.Data; }
};
} // namespace

namespace llvm {
template <> struct DenseMapInfo<StructEmitTy, void> {
  static inline StructEmitTy getEmptyKey() {
    return StructEmitTy::getEmptyKey();
  }
  static inline StructEmitTy getTombstoneKey() {
    return StructEmitTy::getTombstoneKey();
  }

  static unsigned getHashValue(StructEmitTy S) { return S.getHashValue(); }

  static bool isEqual(const StructEmitTy &LHS, const StructEmitTy &RHS) {
    return LHS == RHS;
  }
};
} // namespace llvm

namespace {
// A type to walk through a type/function/etc and emit the metadata, then
// returning the top-level metadata for adding whever it goes.
// See documentation at llvm/llvm/docs/Intel/DTrans/dtrans-opaque-pointers.rst
class DTransInfoGenerator {
  llvm::LLVMContext &Ctx;
  CodeGenModule &CGM;
  llvm::SmallSetVector<StructEmitTy, 32> *AlreadyVisited = nullptr;
  llvm::SmallSetVector<StructEmitTy, 32> *ToBeVisited = nullptr;

  // The types for VFPtr and VBPtr are fixed, as you can see in
  // CGRecordLowering::accumulateVPtrs. the VFPtr is always going to be a
  // i32(...)**.
  llvm::MDNode *CreateVFPtr();
  // The types for VBPtr and VBPtr are fixed, as you can see in
  // CGRecordLowering::accumulateVPtrs. the VBPtr is always going to be a
  // i32*.
  llvm::MDNode *CreateVBPtr();

  // Take a type that is suspected to be an array padding type and create a
  // corresponding clang-type from it so that we properly emit the metadata for
  // it later.
  QualType FixupPaddingType(llvm::Type *LLVMType);

  // Create the metadata for an Array type, provides metadata at the same level
  // as CreateTypeMD.
  llvm::MDNode *CreateArrayTypeMD(QualType ClangType, llvm::Type *LLVMType,
                                  const Expr *InitExpr);

  // Generalized case for Function Metadata that can work for call-types as
  // well.
  llvm::MDNode *CreateFunctionTypeMD(const CGFunctionInfo *CallInfo,
                                     llvm::Type *LLVMType);

  // Wrapper for the function that creates the metadata for function types.
  // Creates metadata at the same level as CreateTypeMD.
  llvm::MDNode *CreateFunctionTypeMD(QualType ClangType, llvm::Type *LLVMType);

  // Creates metadata for an unnamed struct (AKA, a literal struct type).
  // These are really only creatable as types the CFE makes up during
  // Calling-convention generation. Actual C/C++ unnamed types are given a
  // made-up name by the CFE, so these are types that don't need/have ANY
  // linkage requirements, including internal.
  llvm::MDNode *CreateLiteralType(llvm::ArrayRef<QualType> ClangTys,
                                  llvm::ArrayRef<llvm::Type *> LLVMTys);

  // Create the metadata for a vector type, provides metadata at the same level
  // as CreateTypeMD.
  llvm::MDNode *CreateVectorTypeMD(QualType ClangType, llvm::Type *LLVMType);

  // A function to create the metadata for a struct type, which is a fairly
  // complex operation.
  llvm::Metadata *CreateStructMD(QualType ClangType, llvm::Type *LLVMType,
                                 const Expr *InitExpr);

  // A generalized function to correctly generate a metadata chain for an
  // individual type element.  By the time it gets HERE, it should have been
  // stripped of pointers.
  llvm::Metadata *CreateElementMD(QualType ClangType, llvm::Type *LLVMType,
                                  const Expr *InitExpr);

  // Create the metadata for a normal QualType/LLVMType, used particularly for
  // 'fields' of the root-struct layout types we care about.  This is the most
  // generalized/structured portion of the metadata generation.
  llvm::MDNode *CreateTypeMD(QualType ClangType, llvm::Type *LLVMType,
                             const Expr *InitExpr, bool IsABase = false);

  void AddToBeVisited(QualType Ty, bool IsBase) {
    if (!ToBeVisited)
      return;
    if (const RecordDecl *RD = Ty->getAsRecordDecl()) {
      // The '.base' version in IR only happens for a polymorphic type, but any
      // type we name we ALWAYS emit the 'normal' type.
      RD = cast<RecordDecl>(RD->getCanonicalDecl());
      StructEmitTy Emit{RD, StructEmitTy::Standard};
      if (!AlreadyVisited->contains(Emit))
        ToBeVisited->insert(Emit);

      // If this is polymorphic and used as a base, add the '.base' version as
      // well.
      if (const CXXRecordDecl *CXXRD = Ty->getAsCXXRecordDecl()) {
        StructEmitTy BaseEmit{RD, StructEmitTy::Base};
        if (IsBase && !AlreadyVisited->contains(BaseEmit))
          ToBeVisited->insert(BaseEmit);
      }
    }
  }

public:
  DTransInfoGenerator(llvm::LLVMContext &Ctx, CodeGenModule &CGM)
      : Ctx(Ctx), CGM(CGM) {}

  DTransInfoGenerator(llvm::LLVMContext &Ctx, CodeGenModule &CGM,
                      llvm::SmallSetVector<StructEmitTy, 32> *AlreadyVisited,
                      llvm::SmallSetVector<StructEmitTy, 32> *ToBeVisited)
      : Ctx(Ctx), CGM(CGM), AlreadyVisited(AlreadyVisited),
        ToBeVisited(ToBeVisited) {}

  // Add a globally used type the metadata, and return the root node for this
  // type's metadata.
  llvm::MDNode *AddType(const RecordDecl *RD, llvm::StructType *ST);

// Add the DTrans metadata for a type-info C++ type.
  llvm::MDNode *AddTypeInfo(ArrayRef<llvm::Constant *> Fields);

  // Add the information for a type's VTable.
  llvm::MDNode *AddVTable(const VTableLayout &Layout);

  // Add metadata for a field, used by instruction and adding function type.
  llvm::MDNode *AddFieldInfo(QualType ClangType, llvm::Type *LLVMType,
                               const Expr *Init = nullptr) {
    return CreateTypeMD(ClangType, LLVMType, Init);
  }

  // Adds a new literal type, providing the internal clang and llvm types.
  llvm::MDNode *AddLiteralType(llvm::ArrayRef<QualType> ClangTys,
                               llvm::ArrayRef<llvm::Type *> LLVMTys) {
    return CreateLiteralType(ClangTys, LLVMTys);
  }

  // Add information for a function call.
  llvm::MDNode *AddFunctionCallInfo(const CGFunctionInfo *CallInfo,
                                    llvm::Type *LLVMType) {
    return CreateFunctionTypeMD(CallInfo, LLVMType);
  }
};
} // namespace


void CodeGenModule::EmitIntelDTransMetadata() {
  if (!getCodeGenOpts().EmitDTransInfo)
    return;

  // Just the type finder itself isn't enough to determine whether we need to
  // emit a type, we have to make sure we emit all the types that we end up
  // seeing while visiting those types. Store a list of those we've already
  // visited so we don't try to double-visit.  By storing these, DTransTypes
  // needs to remain unmodified so these don't get invalidated, so make sure we
  // work off copies after this.
  // ToBeVisited needs to be a deterministic container, but Visited does not.
  llvm::SmallSetVector<StructEmitTy, 32> Visited;
  llvm::SmallSetVector<StructEmitTy, 32> ToBeVisited;

  llvm::TypeFinder TFinder;
  TFinder.run(getModule(), /*onlyNamed*/ false, /*IncludeMD*/ true);

  for (const auto &Type : DTransTypes) {
    assert(Type.second.size() <= 2 && "More than two representations?");
    for (unsigned I = 0; I < Type.second.size(); ++I) {
      if (llvm::is_contained(TFinder, Type.second[I])) {
        ToBeVisited.insert(
            {Type.first, I ? StructEmitTy::Base : StructEmitTy::Standard});
      }
    }
  }

  llvm::NamedMDNode *DTransRootMD =
      TheModule.getOrInsertNamedMetadata("intel.dtrans.types");
  llvm::LLVMContext &Ctx = TheModule.getContext();
  DTransInfoGenerator Generator(Ctx, *this, &Visited, &ToBeVisited);

  // Loop through until we have no additional records added.
  while (!ToBeVisited.empty()) {
    // Work off of a copy, so we can modify this as the Generator finds new
    // types.
    llvm::SmallSetVector<StructEmitTy, 32> CurToBeVisited;
    std::swap(CurToBeVisited, ToBeVisited);
    Visited.insert(CurToBeVisited.begin(), CurToBeVisited.end());

    for (StructEmitTy Emit : CurToBeVisited) {
      // For some specific edge cases, there isn't a '.base' for 
      // types used as a base, particularly polymorphic types that don't have
      // any data members. Don't try to emit them, since one doesn't exist.
      if (Emit.isBase() && DTransTypes[Emit.getRecordDecl()].size() < 2)
        continue;
      DTransRootMD->addOperand(
          Generator.AddType(Emit.getRecordDecl(),
                            DTransTypes[Emit.getRecordDecl()][Emit.isBase()]));
    }
  }
}

llvm::Function *CodeGenModule::addDTransInfoToFunc(GlobalDecl GD,
                                                   StringRef MangledName,
                                                   llvm::FunctionType *FT,
                                                   llvm::Function *Func) {
  if (!getCodeGenOpts().EmitDTransInfo || !GD)
    return Func;

  assert(MangledName.size() > 0 && "No mangled name?");
  CodeGenTypes::DTransFuncInfo FuncInfo;
  // Fill in the dtrans func info.
  llvm::FunctionType *FT2 = getTypes().GetFunctionType(GD, &FuncInfo);
  assert((!cast<FunctionDecl>(GD.getDecl())->hasPrototype() || FT2 == FT) &&
         "Generated different function type?");
  (void)FT2;

  return addDTransInfoToFunc(FuncInfo, FT, Func);
}

llvm::Function *
CodeGenModule::addDTransInfoToFunc(const CodeGenTypes::DTransFuncInfo &FuncInfo,
                                   llvm::FunctionType *FT,
                                   llvm::Function *Func) {
  if (!getCodeGenOpts().EmitDTransInfo || !Func)
    return Func;

  llvm::LLVMContext &Ctx = TheModule.getContext();
  llvm::SmallVector<llvm::Metadata*, 4> Attachments;

  DTransInfoGenerator Generator(Ctx, *this);

  unsigned NodeIdx = 0;
  if (!FuncInfo.ResultTypes[0].isNull()) {
    if (Func->getReturnType()->isPointerTy()) {
      Func->addAttributeAtIndex(0, llvm::Attribute::get(Ctx, "intel_dtrans_func_index",
                                                 std::to_string(++NodeIdx)));
      Attachments.push_back(Generator.AddFieldInfo(FuncInfo.ResultTypes[0],
                                                   Func->getReturnType()));
    } else if (Func->getReturnType()->isStructTy()) {
      // Handle the anonymous struct return type when decomposed/recomposed.
      llvm::StructType *ST = cast<llvm::StructType>(Func->getReturnType());
      assert (ST->getNumElements() == 2 && "Larger than 2 struct ty?");

      if (ST->elements()[0]->isPointerTy() ||
          ST->elements()[1]->isPointerTy()) {
        Func->addAttributeAtIndex(0,
                           llvm::Attribute::get(Ctx, "intel_dtrans_func_index",
                                                std::to_string(++NodeIdx)));
        Attachments.push_back(
            Generator.AddLiteralType(FuncInfo.ResultTypes, ST->elements()));
      }
    }
  }

  unsigned Idx = 0;
  for (QualType Ty : FuncInfo.Params) {
    if (!Ty.isNull() && Func->getArg(Idx)->getType()->isPointerTy()) {
      Func->addParamAttr(Idx,
                         llvm::Attribute::get(Ctx, "intel_dtrans_func_index",
                                              std::to_string(++NodeIdx)));
      Attachments.push_back(
          Generator.AddFieldInfo(Ty, Func->getArg(Idx)->getType()));
    }
    ++Idx;
  }

  if (!Attachments.empty()) {
    llvm::MDTuple *MDTypes = llvm::MDTuple::getDistinct(Ctx, Attachments);
    Func->setMetadata("intel.dtrans.func.type", MDTypes);
  }
  return Func;
}

Address CodeGenModule::addDTransInfoToMemTemp(QualType Ty, Address Addr) {
  if (!getCodeGenOpts().EmitDTransInfo)
    return Addr;

  if (!Ty->isPointerType() && !Ty->isArrayType() && !Ty->isReferenceType())
    return Addr;

  llvm::LLVMContext &Ctx = TheModule.getContext();
  llvm::Instruction *Alloca = cast<llvm::Instruction>(Addr.getPointer());

  DTransInfoGenerator Generator(Ctx, *this);

  Alloca->setMetadata("intel_dtrans_type",
                      Generator.AddFieldInfo(Ty, Addr.getElementType()));

  return Addr;
}

llvm::GlobalVariable *CodeGenModule::addDTransInfoToGlobal(
    const VarDecl *VD, llvm::GlobalVariable *GV, llvm::Type *LLVMType) {
  return addDTransInfoToGlobal(VD->getType(), VD->getAnyInitializer(), GV,
                               LLVMType);
}

llvm::GlobalVariable *
CodeGenModule::addDTransInfoToGlobal(QualType Ty, const Expr *Init,
                                     llvm::GlobalVariable *GV,
                                     llvm::Type *LLVMType) {
  if (!getCodeGenOpts().EmitDTransInfo)
    return GV;

  if (!Ty->isPointerType() && !Ty->isReferenceType() && !Ty->isArrayType())
    return GV;

  llvm::LLVMContext &Ctx = TheModule.getContext();
  DTransInfoGenerator Generator(Ctx, *this);
  GV->setMetadata("intel_dtrans_type",
                  Generator.AddFieldInfo(Ty, LLVMType, Init));
  return GV;
}

llvm::GlobalVariable *
CodeGenModule::addDTransVTableInfo(llvm::GlobalVariable *GV,
                                   const VTableLayout &Layout) {
  if (!getCodeGenOpts().EmitDTransInfo)
    return GV;

  llvm::LLVMContext &Ctx = TheModule.getContext();
  DTransInfoGenerator Generator(Ctx, *this);
    GV->setMetadata("intel_dtrans_type", Generator.AddVTable(Layout));
    return GV;
}

// Add DTrans metadata information for the C++ type-info types.
llvm::GlobalVariable *CodeGenModule::addDTransTypeInfo(
    llvm::GlobalVariable *GV, const SmallVectorImpl<llvm::Constant *> &Fields) {
  if (!getCodeGenOpts().EmitDTransInfo)
    return GV;

  llvm::LLVMContext &Ctx = TheModule.getContext();
  DTransInfoGenerator Generator(Ctx, *this);

  GV->setMetadata("intel_dtrans_type",
                  Generator.AddTypeInfo(Fields));
  return GV;
}

llvm::CallBase *
CodeGenModule::addDTransIndirectCallInfo(llvm::CallBase *CI,
                                         const CGFunctionInfo &CallInfo) {
  if (!getCodeGenOpts().EmitDTransInfo)
    return CI;

  llvm::LLVMContext &Ctx = TheModule.getContext();
  DTransInfoGenerator Generator(Ctx, *this);
  CI->setMetadata("intel_dtrans_type", Generator.AddFunctionCallInfo(
                                           &CallInfo, CI->getFunctionType()));
  return CI;
}

namespace {
llvm::MDNode *DTransInfoGenerator::AddType(const RecordDecl *RD,
                                           llvm::StructType *ST) {
  // Named structures are of the form:
  // For Opaque Structs(size field is -1):
  // !{!"S", %struct.foo zeroinitializer, i32 -1 }
  // For empty structs (size field is 0):
  // !{!"S", %struct.empty zeroinitializer, i32 0 }
  // For the rest, there are N (field count) metadata links, representing each
  // field:
  // !{!"S", %struct.empty zeroinitializer, i32 N, !2, !3, !4,... }
  llvm::SmallVector<llvm::Metadata *> MD;
  MD.push_back(llvm::MDString::get(Ctx, "S"));
  MD.push_back(
      llvm::ConstantAsMetadata::get(llvm::ConstantAggregateZero::get(ST)));
  MD.push_back(llvm::ConstantAsMetadata::get(
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctx),
                             ST->isOpaque() ? -1 : ST->getNumElements())));

  if (ST->getNumElements()) {
    const CGRecordLayout &Layout = CGM.getTypes().getCGRecordLayout(RD);
    if (RD->isUnion()) {
      // Unions can have padding, and can end up being JUST an array with no
      // real field, so we only fill it in if we know it.
      if (const FieldDecl *UD = Layout.getUnionDecl()) {
        MD.push_back(
            CreateTypeMD(UD->getType(), ST->getElementType(0), nullptr));

        if (ST->getNumElements() == 2) {
          llvm::Type *LLVMType = ST->getElementType(1);
          QualType ClangType = FixupPaddingType(LLVMType);
          MD.push_back(CreateTypeMD(ClangType, LLVMType, nullptr));
        }

      }
    } else {
      for (unsigned Idx = 0, E = ST->getNumElements(); Idx < E; ++Idx) {
        if (Layout.IsIdxVFPtr(Idx))
          MD.push_back(CreateVFPtr());
        else if (Layout.IsIdxVBPtr(Idx))
          MD.push_back(CreateVBPtr());
        else {
          QualType ClangType = Layout.getTypeOfLLVMFieldNum(Idx);
          bool IsABase = Layout.IsFieldNumBase(Idx);
          llvm::Type *LLVMType = ST->getElementType(Idx);
          // If ClangType is null here, this struct element is either padding,
          // or a bitfield. Padding is either an array of i8, or a single i8.
          // We can just fixup the clang-type, since that is all that would be
          // emitted otherwise. The integral case should just handle the
          // bitfields.
          if (ClangType.isNull())
            ClangType = FixupPaddingType(LLVMType);

          ClangType = ClangType.getCanonicalType();

          // A group of bitfields may ALSO be represented as an llvm array of
          // integers, so if this is the case, just do a 'padding' fixup, so the
          // integral case should properly cover this.
          // We don't know of any cases a non-field could be an array in this
          // case, nor a non-bitfield.
          if (ClangType->isIntegralOrEnumerationType() &&
              LLVMType->isArrayTy() &&
              LLVMType->getArrayElementType()->isIntegerTy()) {
            assert(
                Layout.getFieldOfLLVMFieldNum(Idx) &&
                Layout.getFieldOfLLVMFieldNum(Idx)->isBitField() &&
                "Unknown LLVM Array type representing a Clang integral type");
            ClangType = FixupPaddingType(LLVMType);
          }

          MD.push_back(CreateTypeMD(ClangType, LLVMType, nullptr, IsABase));
        }
      }
    }
  }

  // This assert is likely to cause a good amount of regressions, so allow the
  // backend folks to disable it while doing their work so they aren't blocked
  // during their testing.
  if (!CGM.getCodeGenOpts().DisableDTransAsserts) {
    // For these types, the format of the dtrans info is:
    // "!{!"S", <struct zero init>, i32 <NumFields>, <List of field MD>}.
    // We want to check that the number of metadata nodes generated for fields
    // matches the number of elements in the struct, so we do num-elements + 3,
    // since the 3 non-field metadata nodes shouldn't count against this limit.
    assert(ST->getNumElements() + 3 == MD.size() &&
           "Incorrect number of struct fields generated");
  }
  return llvm::MDNode::get(Ctx, MD);
}
// The types for VFPtr and VBPtr are fixed, as you can see in
// CGRecordLowering::accumulateVPtrs. the VFPtr is always going to be a
// i32(...)**.
llvm::MDNode *DTransInfoGenerator::CreateVFPtr() {
  // Function returns an i32.
  llvm::MDNode *FuncRet = llvm::MDNode::get(
      Ctx, {llvm::ConstantAsMetadata::get(
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctx), 0)),
            llvm::ConstantAsMetadata::get(
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctx), 0))});

  // Function type is !"F", i1 true, i32 0, Ret.
  // 'true' means 'varargs' here.
  llvm::MDNode *Func = llvm::MDNode::get(
      Ctx, {llvm::MDString::get(Ctx, "F"),
            llvm::ConstantAsMetadata::get(
                llvm::ConstantInt::get(llvm::Type::getInt1Ty(Ctx), 1)),
            llvm::ConstantAsMetadata::get(
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctx), 0)),
            FuncRet});

  // Param value is the MDNode with Func, i32 2 (for **).
  return llvm::MDNode::get(
      Ctx, {Func, llvm::ConstantAsMetadata::get(
                      llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctx), 2))});
}
// The types for VBPtr and VBPtr are fixed, as you can see in
// CGRecordLowering::accumulateVPtrs. the VBPtr is always going to be a
// i32*.
llvm::MDNode *DTransInfoGenerator::CreateVBPtr() {
  std::array<llvm::Metadata *, 2> TopLevelMD;
  TopLevelMD[0] = llvm::ConstantAsMetadata::get(
      llvm::Constant::getNullValue(llvm::Type::getInt32Ty(Ctx)));
  TopLevelMD[1] = llvm::ConstantAsMetadata::get(
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctx), 1));
  return llvm::MDNode::get(Ctx, TopLevelMD);
}

// Take a type that is suspected to be an array padding type and create a
// corresponding clang-type from it so that we properly emit the metadata for
// it later.
QualType DTransInfoGenerator::FixupPaddingType(llvm::Type *LLVMType) {
  if (LLVMType->isIntegerTy()) {
    // Padding is just an integer, so we can just correct this to the right
    // sized unsigned type.  It doesn't matter if the size is different/wrong,
    // the backend doesn't care.
    return CGM.getContext().getIntTypeForBitwidth(
        LLVMType->getIntegerBitWidth(), /*signed*/ 0);
  } else if (LLVMType->isArrayTy() &&
             LLVMType->getArrayElementType()->isIntegerTy()) {
    // This is a padding array.  Again, we just pretend this is an array of a
    // known-width integer.
    QualType ElemTy = CGM.getContext().getIntTypeForBitwidth(
        LLVMType->getArrayElementType()->getIntegerBitWidth(),
        /*signed*/ 0);
    return CGM.getContext().getConstantArrayType(
        ElemTy, llvm::APInt(64, LLVMType->getArrayNumElements()), nullptr,
        clang::ArrayType::Normal, 0);
  }
  llvm_unreachable("Unknown padding type");
}

// Create the metadata for a normal QualType/LLVMType, used particularly for
// 'fields' of the root-struct layout types we care about.  This is the most
// generalized/structured portion of the metadata generation.
llvm::MDNode *DTransInfoGenerator::CreateTypeMD(QualType ClangType,
                                                llvm::Type *LLVMType,
                                                const Expr *InitExpr,
                                                bool IsABase) {
  llvm::SmallVector<llvm::Metadata *> MD;
  // Format for these fields is:
  // !{<type> zeroinitializer, i32 <pointer level> }
  // where <pointer level> is the 'pointer depth' to the type. For more
  // complicated types, the zero-initializer can be a substituted by another
  // metadata type.

  unsigned PointerCount = 0;
  switch (LLVMType->getTypeID()) {
  case llvm::Type::ScalableVectorTyID:
    llvm_unreachable("Unsupported type ScalableVectorTyID");
    break;
    // Array, Vector, and Function Metadata  can't be ONLY in CreateElementMD,
    // because they don't follow the 'pointer-count' logic that the rest do.
  case llvm::Type::ArrayTyID:
    return CreateArrayTypeMD(ClangType, LLVMType, InitExpr);
  case llvm::Type::FixedVectorTyID:
    return CreateVectorTypeMD(ClangType, LLVMType);
   case llvm::Type::FunctionTyID:
    return CreateFunctionTypeMD(ClangType, LLVMType);
  case llvm::Type::PointerTyID: {

    // Pointer handling is the most particular/important here, we cannot look
    // into the LLVMType's element type, since the opaque_ptr makes that go
    // away.
    assert(!ClangType.isNull() && "Pointers should always have a field decl");
    assert((ClangType->isPointerType() || ClangType->isReferenceType() ||
            ClangType->isVariableArrayType() || ClangType->isNullPtrType() ||
            (ClangType->isMemberFunctionPointerType() &&
             CGM.getTriple().isWindowsMSVCEnvironment())) &&
           "Not a pointer/vla/reference type?");

    // We need to do this as a separate step from the loop below, since the
    // clang::Type methods only work with Pointers and not references
    // (despite ALSO being named getPointeeType).  Additionally, references
    // can only be the 'top' level thanks to the rules of C++ (you cannot
    // have a pointer to a reference type), so it is safe to do it here.
    // Also, r-value references have the same IR as l-value references, so
    // this should be sufficient.
    if (const auto *RT = ClangType->getAs<ReferenceType>()) {
      PointerCount++;
      ClangType = RT->getPointeeType();
    }
    // VLAs are just represented by the internal type, so we just need to
    // get the element type and count on the rest to 'just work'.
    if (ClangType->isVariableArrayType()) {
      ClangType = CGM.getContext().getBaseElementType(ClangType);
    }

    // Unroll all the pointers, and increment the PointerCount as we go.
    while (ClangType->isPointerType()) {
      PointerCount++;
      ClangType = ClangType->getPointeeType();
    }

    // Creating a pointer to std::nullptr_t is pretty nonsense, but possible.
    if (ClangType->isNullPtrType()) {
      ClangType = CGM.getContext().VoidTy;
      PointerCount++;
    }

    if (ClangType->isMemberFunctionPointerType() &&
        CGM.getTriple().isWindowsMSVCEnvironment()) {
      // Pointer-to-Member Functions are represented on windows as either an
      // i8*, or as a struct of {i8* followed by a number of i32s}.  This code
      // path covers the former by just emitting the i8 pointer info.
      MD.push_back(CreateElementMD(CGM.getContext().CharTy,
                                   llvm::Type::getInt8Ty(Ctx), InitExpr));
      MD.push_back(llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(
          llvm::Type::getInt32Ty(Ctx), 1 + PointerCount)));
      return llvm::MDNode::get(Ctx, MD);
    }

    // Now re-make the LLVM Type from the Clang type.
    LLVMType = CGM.getTypes().ConvertTypeForMem(ClangType);
  }
    LLVM_FALLTHROUGH;
  default: {
    // Handle the primative LLVM-IR types.  These types should just be
    // zero-init versions of the IR types.
    AddToBeVisited(ClangType, IsABase);
    MD.push_back(CreateElementMD(ClangType, LLVMType, InitExpr));
    MD.push_back(llvm::ConstantAsMetadata::get(
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctx), PointerCount)));
  }
  }

  return llvm::MDNode::get(Ctx, MD);
}

static bool IsPaddingCandidate(CodeGenModule &CGM, llvm::Type *Ty) {
  return Ty->isArrayTy() &&
         Ty->getArrayElementType()->isIntegerTy(
             CGM.getContext().getTypeSize(CGM.getContext().CharTy));
}

static bool IsPaddingCandidate(CodeGenModule &CGM, llvm::Type *Ty,
                               uint64_t Size) {
  return IsPaddingCandidate(CGM, Ty) && Ty->getArrayNumElements() == Size;
}

static bool IsPaddingCandidate(CodeGenModule &CGM, QualType FieldTy,
                               uint64_t Size) {
  const ConstantArrayType *FieldArrayTy =
      CGM.getContext().getAsConstantArrayType(FieldTy);

  return FieldArrayTy &&
         CGM.getContext().getTypeSize(FieldArrayTy->getElementType()) ==
             CGM.getContext().getTypeSize(CGM.getContext().CharTy) &&
         FieldArrayTy->getSize() == Size;
}

// Try to figure out if this is padding after a bitfield. We know it is only
// padding if it is an i8 array after the bitfield. This is currently imperfect
// (in a way that seemingly doesn't appear in any reproducers) for complicated
// cases with arrays of char that match the padding size. The actual way this
// padding is generated, particularly on Windows, seems to be based on the size,
// and underlying type of the bitfield, plus the size of the next field.
static bool IsPaddingAfterBitfield(CodeGenModule &CGM, llvm::StructType *ST,
                                   llvm::Type *LLVMPadding,
                                   unsigned LLVMIdx,
                                   const RecordDecl *RD,
                                   unsigned NextFieldClangIdx) {
  // Only padding if the llvm type is an array of i8.
  if (!IsPaddingCandidate(CGM, LLVMPadding))
    return false;

  uint64_t PaddingArraySize = LLVMPadding->getArrayNumElements();

  // Only this category of padding if the bitfield is not the last field in the
  // struct. It might still be padding, but it would be struct-level padding at
  // that point.
  if (NextFieldClangIdx >= std::distance(RD->field_begin(), RD->field_end()))
    return false;

  RecordDecl::field_iterator Itr = RD->field_begin();
  std::advance(Itr, NextFieldClangIdx);

  // If the next field is not a constant array, or the sizes don't match, we know this is padding.
  if (!IsPaddingCandidate(CGM, (*Itr)->getType(), PaddingArraySize))
    return true;

  // Count the number of potential-padding llvm arrays, and the number of clang
  // array-types that might be padding.  If there are the same number, it
  // probably isn't padding.  If they AREN'T the same, assume this is padding
  // here. The assumption doesn't really matter, since the metadata ends up
  // being the same anyway.
  unsigned LLVMCandidates = 0;
  unsigned ClangCandidates = 0;

  for (; LLVMIdx < ST->getNumElements() &&
         IsPaddingCandidate(CGM, ST->getElementType(LLVMIdx), PaddingArraySize);
       ++LLVMIdx) {
    ++LLVMCandidates;
  }

  for (; Itr != RD->field_end() &&
         IsPaddingCandidate(CGM, (*Itr)->getType(), PaddingArraySize);
       ++Itr) {
    ++ClangCandidates;
  }

  return LLVMCandidates > ClangCandidates;
}

// A generalized function to correctly generate a metadata chain for an
// individual type element.  By the time it gets HERE, it should have been
// stripped of pointers.
llvm::Metadata *DTransInfoGenerator::CreateElementMD(QualType ClangType,
                                                     llvm::Type *LLVMType,
                                                     const Expr *InitExpr) {
  // FIXME: Doesn't handle vector-type pointees, but that doesn't seem to show
  // up in any of the reproducers.  When we're sure it is necessary, we can add
  // that in.
  assert(!ClangType->isPointerType() && !ClangType->isVectorType() &&
         "Shouldn't get here");

  switch (LLVMType->getTypeID()) {
  case llvm::Type::FunctionTyID:
    return CreateFunctionTypeMD(ClangType, LLVMType);
  case llvm::Type::ArrayTyID:
    return CreateArrayTypeMD(ClangType, LLVMType, InitExpr);
  case llvm::Type::StructTyID:
    return CreateStructMD(ClangType, LLVMType, InitExpr);
  case llvm::Type::VoidTyID:
    // Void encoding is: !{!"void", i32 <pointer level> }
    // Since we're treating this as a normal 'type', the pointer level gets
    // taken care of by the caller.
    return llvm::MDString::get(Ctx, "void");
  default:
  return llvm::ConstantAsMetadata::get(llvm::Constant::getNullValue(LLVMType));
  }
}
  // A function to create the metadata for a struct type, which is a fairly
  // complex operation.
llvm::Metadata *DTransInfoGenerator::CreateStructMD(QualType ClangType,
                                                    llvm::Type *LLVMType,
                                                    const Expr *InitExpr) {
  llvm::StructType *ST = cast<llvm::StructType>(LLVMType);
  ClangType = ClangType.getCanonicalType();

  // Non literal struct types are really simple, they are just a
  // zero-initialized version of the struct. The contents are decomposed
  // separately thanks to these types being emitted at top level.
  if (!ST->isLiteral())
    return llvm::ConstantAsMetadata::get(llvm::ConstantAggregateZero::get(ST));

  // Literal struct type MD: !{!"L", i32 <numElem>, !MDNodefield1, !MDNodefield2, ...}
  llvm::SmallVector<llvm::Metadata *> LitMD;
  LitMD.push_back(llvm::MDString::get(Ctx, "L"));
  LitMD.push_back(llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(
      llvm::Type::getInt32Ty(Ctx), ST->getNumElements())));

  if (const auto *RD = ClangType->getAsRecordDecl()) {
    if (RD->isUnion()) {
      // Can be 1 (a union field), or 2 (a union field + padding).
      assert(ST->getNumElements() <= 2 &&
             "Union represented by multiple fields?");
      const auto *ILE = cast<InitListExpr>(InitExpr);
      const FieldDecl *InitedField = ILE->getInitializedFieldInUnion();
      assert(InitedField && "Union wihtout initialized field?");
      LitMD.push_back(CreateTypeMD(InitedField->getType(),
                                      ST->getElementType(0), ILE->getInit(0)));

      if (ST->getNumElements() == 2) {
        QualType PaddingTy =
            FixupPaddingType(ST->getElementType(1));
        LitMD.push_back(
            CreateTypeMD(PaddingTy, ST->getElementType(1), nullptr));
      }
    } else {
      // The layout object doesn't really map correctly in the global layout
      // case (as bitfields at least are broken down?), so we cannot use it
      // for anything. We need to just try to manage it in another way,
      // here.
      unsigned Idx = 0;
      unsigned ClangIdx = 0;
      for (const auto *FD : RD->fields()) {
        const Expr *CurInit = nullptr;
        if (const auto *ILE = dyn_cast_or_null<InitListExpr>(InitExpr))
          CurInit = ILE->getInit(ClangIdx);

        if (FD->isBitField()) {
          // Bitfields for these seem to be broken down into the correct
          // 'chars'.  I'm not sure what happens with various combinations,
          // so I suspect we'll have to deal with this again.
          unsigned Width = FD->getBitWidthValue(CGM.getContext());

          // Bitfields can require padding when stored in a global array before
          // them to get the alignment right, particularly on windows. We know
          // at this point it is an i8 array, so just encode that if we've run
          // across it.
          if (ST->getElementType(Idx)->isArrayTy()) {
            llvm::Type *LLVMPadding = ST->getElementType(Idx);
            assert(LLVMPadding->getArrayElementType()->isIntegerTy(8) &&
                   "Not bitfield leading padding?");
            QualType ClangPadding = FixupPaddingType(LLVMPadding);
            LitMD.push_back(CreateTypeMD(ClangPadding, LLVMPadding, CurInit));
            ++Idx;
          }

          for (unsigned Cur = 0; Cur < Width; Cur += 8) {
            LitMD.push_back(CreateTypeMD(CGM.getContext().CharTy,
                                            ST->getElementType(Idx), CurInit));
            ++Idx;
          }
          ++ClangIdx;

          // If there is a remaining LLVM field, there is a possibility for it
          // to be padding between this bitfield and the next field.
          if (ST->getNumElements() > Idx) {
            llvm::Type *LLVMPadding = ST->getElementType(Idx);
            if (IsPaddingAfterBitfield(CGM, ST, LLVMPadding, Idx, RD,
                                       ClangIdx)) {
              QualType ClangPadding = FixupPaddingType(LLVMPadding);
              LitMD.push_back(CreateTypeMD(ClangPadding, LLVMPadding, CurInit));
              ++Idx;
            }
          }
        } else {
          LitMD.push_back(
              CreateTypeMD(FD->getType(), ST->getElementType(Idx), CurInit));
          ++ClangIdx;
          ++Idx;
        }
      }

      // Handle tail padding. No clang field to do it with, so just emit it.
      if (ST->getNumElements() > Idx) {
        assert(Idx + 1 == ST->getNumElements() && "More than 1 padding field?");
        llvm::Type *LLVMPadding = ST->getElementType(Idx);
        assert(LLVMPadding->getArrayElementType()->isIntegerTy(8) &&
               "Not bitfield leading padding?");
        QualType ClangPadding = FixupPaddingType(LLVMPadding);
        LitMD.push_back(
            CreateTypeMD(ClangPadding, ST->getElementType(Idx), nullptr));
      }
    }
  } else if (const auto *Cplx = ClangType->getAs<ComplexType>()) {
    LitMD.push_back(
        CreateTypeMD(Cplx->getElementType(), ST->getElementType(0), nullptr));
    LitMD.push_back(
        CreateTypeMD(Cplx->getElementType(), ST->getElementType(1), nullptr));
  } else if (const auto *Arr =
                 CGM.getContext().getAsConstantArrayType(ClangType)) {
    // This is an odd situation where the array is broken up into
    // sub-elements/arrays to better work with elements that are
    // initialized.
    const auto *ILE = dyn_cast_or_null<InitListExpr>(InitExpr);

    for (unsigned I = 0, E = ST->getNumElements(); I < E; ++I) {
      const Expr *CurInit = nullptr;
      if (ILE && ILE->getNumInits() > I) {
        CurInit = ILE->getInit(I);
      } else if (ILE) {
        assert(ILE->hasArrayFiller() && "Unknown init list type");
        CurInit = ILE->getArrayFiller();
      }

      if (ST->getElementType(I)->isArrayTy()) {
        QualType SmallerArray = CGM.getContext().getConstantArrayType(
            Arr->getElementType(),
            llvm::APInt(64, ST->getElementType(I)->getArrayNumElements()),
            Arr->getSizeExpr(), Arr->getSizeModifier(),
            Arr->getIndexTypeQualifiers().getAsOpaqueValue());

        LitMD.push_back(
            CreateTypeMD(SmallerArray, ST->getElementType(I), CurInit));
      } else {
        LitMD.push_back(CreateTypeMD(Arr->getElementType(),
                                        ST->getElementType(I), CurInit));
      }
    }
  } else if (const auto *FPtr = ClangType->getAs<FunctionProtoType>()) {
    // The only confirmed time a literal struct is used to represent a
    // FunctionProtoType is as an empty literal struct. This seems to happen
    // when this is a function pointer with an incomplete record type in its
    // parameters list.
    assert(ST->getNumElements() == 0  && "Don't know how to handle this yet");
    // As there are no fields, there is nothing to do here.
  } else {
    assert(ClangType->getAs<MemberPointerType>() &&
           "Unknown LLVM Literal Struct type");

    if (CGM.getTriple().isWindowsMSVCEnvironment()) {
      // Pointer-to-member-functions are represented on windows as either an
      // i8*, or as a struct of {i8* followed by a number of i32s}.  This code
      // path covers the latter by just emitting the correct info.
      assert(ST->getElementType(0)->isPointerTy() &&
             "Windows Member pointer type first element not pointer?");

      QualType PtrElemTy =
          CGM.getContext().getPointerType(CGM.getContext().CharTy);
      LitMD.push_back(CreateTypeMD(PtrElemTy, ST->getElementType(0), nullptr));

      llvm::Type *EltTy = nullptr;
      for (unsigned I = 1; I < ST->getNumElements(); ++I) {
        assert((I == 1 || ST->getElementType(I) == EltTy) &&
               "Windows member pointer types not all the same?");
        EltTy = ST->getElementType(I);
        assert(EltTy->isIntegerTy() &&
               "Windows member pointer type followed by not integer?");
        QualType IntElemTy = CGM.getContext().getIntTypeForBitwidth(
            EltTy->getIntegerBitWidth(), /*signed*/ 0);

        LitMD.push_back(CreateTypeMD(IntElemTy, EltTy, nullptr));
      }
      return llvm::MDNode::get(Ctx, LitMD);
    }


    // Member pointers get represented as a struct of 2 integer types.
    assert(ST->getNumElements() == 2 &&
           "Member pointer type represented by >2 ints?");
    assert(ST->getElementType(0)->isIntegerTy() &&
           ST->getElementType(0) == ST->getElementType(1) &&
           "Member pointer type not two integers?");

    QualType ElemTy = CGM.getContext().getIntTypeForBitwidth(
        ST->getElementType(0)->getIntegerBitWidth(), /*signed*/ 0);
    LitMD.push_back(CreateTypeMD(ElemTy, ST->getElementType(0), nullptr));
    LitMD.push_back(CreateTypeMD(ElemTy, ST->getElementType(1), nullptr));
    return llvm::MDNode::get(Ctx, LitMD);
  }

  return llvm::MDNode::get(Ctx, LitMD);
}

// Create the metadata for a vector type, provides metadata at the same level
// as CreateTypeMD.
llvm::MDNode *DTransInfoGenerator::CreateVectorTypeMD(QualType ClangType,
                                                      llvm::Type *LLVMType) {
  // Metadata format is: !{!"V", i32 <numElem>, !MDNode }
  assert(LLVMType->isVectorTy() && "Not a vector type?");
  auto *VT = cast<llvm::FixedVectorType>(LLVMType);

  llvm::SmallVector<llvm::Metadata *> VecMD;
  VecMD.push_back(llvm::MDString::get(Ctx, "V"));
  VecMD.push_back(llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(
      llvm::Type::getInt32Ty(Ctx), VT->getNumElements())));

  const VectorType *VTy = cast<VectorType>(ClangType.getTypePtr());
  assert(VTy->getNumElements() == VT->getNumElements() &&
         "Mismatched vector type sizes");

  VecMD.push_back(
      CreateTypeMD(VTy->getElementType(), VT->getElementType(), nullptr));

  return llvm::MDNode::get(Ctx, VecMD);
}

// Creates metadata for an unnamed struct (AKA, a literal struct type).
// These are really only creatable as types the CFE makes up during
// Calling-convention generation. Actual C/C++ unnamed types are given a
// made-up name by the CFE, so these are types that don't need/have ANY
// linkage requirements, including internal.
llvm::MDNode *
DTransInfoGenerator::CreateLiteralType(llvm::ArrayRef<QualType> ClangTys,
                                       llvm::ArrayRef<llvm::Type *> LLVMTys) {
  assert(ClangTys.size() == LLVMTys.size() && "Not the same size?");
  // Format is: !{!"L", i32 <numElem>, !MDNodefield1, !MDNodefield2, ...}

  llvm::SmallVector<llvm::Metadata *> MD;
  MD.push_back(llvm::MDString::get(Ctx, "L"));
  MD.push_back(llvm::ConstantAsMetadata::get(
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctx), ClangTys.size())));

  for (unsigned I = 0, E = ClangTys.size(); I < E; ++I)
    MD.push_back(CreateTypeMD(ClangTys[I], LLVMTys[I], nullptr));

  return llvm::MDNode::get(Ctx, MD);
}

// Wrapper for the function that creates the metadata for function types.
// Creates metadata at the same level as CreateTypeMD.
llvm::MDNode *DTransInfoGenerator::CreateFunctionTypeMD(QualType ClangType,
                                                        llvm::Type *LLVMType) {
  assert(LLVMType->isFunctionTy() && "Not a function type?");
  ClangType = ClangType.getCanonicalType();
  const CGFunctionInfo *FI = nullptr;
  if (const FunctionProtoType *FPT =
          dyn_cast<FunctionProtoType>(ClangType.getTypePtr())) {
    FI = &CGM.getTypes().arrangeFreeFunctionType(
        CanQual<FunctionProtoType>::CreateUnsafe(QualType(FPT, 0)));
  } else {
    const FunctionNoProtoType *FNPT =
        cast<FunctionNoProtoType>(ClangType.getTypePtr());
    FI = &CGM.getTypes().arrangeFreeFunctionType(
        CanQual<FunctionNoProtoType>::CreateUnsafe(QualType(FNPT, 0)));
  }
  return CreateFunctionTypeMD(FI, LLVMType);
}

// Generalized case for Function Metadata that can work for call-types as
// well.
llvm::MDNode *
DTransInfoGenerator::CreateFunctionTypeMD(const CGFunctionInfo *CallInfo,
                                          llvm::Type *LLVMType) {
  // Metadata format is:
  // !{!"F", i1 <isVarArg>, i32 <numParam>, !MDNoderet, !MDNodeparam1, ... }
  assert(LLVMType->isFunctionTy() && "Not a function type?");
  llvm::SmallVector<llvm::Metadata *> FuncMD;
  FuncMD.push_back(llvm::MDString::get(Ctx, "F"));
  FuncMD.push_back(llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(
      llvm::Type::getInt1Ty(Ctx), LLVMType->isFunctionVarArg())));
  FuncMD.push_back(llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(
      llvm::Type::getInt32Ty(Ctx), LLVMType->getFunctionNumParams())));

  // Build Function Info.
  CodeGenTypes::DTransFuncInfo FuncInfo;
  (void)CGM.getTypes().GetFunctionType(*CallInfo, &FuncInfo);

  llvm::FunctionType *FuncTy = cast<llvm::FunctionType>(LLVMType);

  if (FuncInfo.ResultTypes[0].isNull()) {
    // Void return type.
    FuncMD.push_back(
        CreateTypeMD(CGM.getContext().VoidTy, FuncTy->getReturnType(), nullptr));
  } else if (FuncInfo.ResultTypes[1].isNull()) {
    // Only a single result-type, so nothing else we should have to do.
    FuncMD.push_back(
        CreateTypeMD(FuncInfo.ResultTypes[0], FuncTy->getReturnType(), nullptr));
  } else {
    // Literal struct return type.
    llvm::StructType *ST = cast<llvm::StructType>(FuncTy->getReturnType());
    assert(ST->getNumElements() == 2 && "Larger than 2 struct ty?");
    FuncMD.push_back(CreateLiteralType(FuncInfo.ResultTypes, ST->elements()));
  }

  unsigned Idx = 0;
  for (QualType &Ty : FuncInfo.Params) {
    FuncMD.push_back(CreateTypeMD(Ty, FuncTy->getParamType(Idx), nullptr));
    ++Idx;
  }

  return llvm::MDNode::get(Ctx, FuncMD);
}

// Create the metadata for an Array type, provides metadata at the same level
// as CreateTypeMD.
llvm::MDNode *DTransInfoGenerator::CreateArrayTypeMD(QualType ClangType,
                                                     llvm::Type *LLVMType,
                                                     const Expr *InitExpr) {
  // Array MD Format:
  // !{!"A", i32 <numElem>, !MDNode }
  assert(!ClangType.isNull() && ClangType->isArrayType() &&
         "Clang Type not an array type?");
  assert(LLVMType->isArrayTy() && "LLVMType Not an array type?");
  llvm::SmallVector<llvm::Metadata *> ArrMD;

  ArrMD.push_back(llvm::MDString::get(Ctx, "A"));
  ArrMD.push_back(llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(
      llvm::Type::getInt32Ty(Ctx), LLVMType->getArrayNumElements())));

  const Expr *CurInit = nullptr;

  if (const auto *ILE = dyn_cast_or_null<InitListExpr>(InitExpr)) {
    // Since we only care about the TYPE of the initializer, and an array is
    // going to be initialized with the same type throughout, we don't care
    // about anything other than the 1st initializer.
    if (ILE->getNumInits()) {
      CurInit = ILE->getInit(0);
    } else if (ILE->hasArrayFiller()) {
      CurInit = ILE->getArrayFiller();
    }
    // If there is no initializer, this is likely a zero-length array, which
    // just uses the type directly rather than a level of decomposition. So we
    // should already have the correct element type, and don't need the
    // initializer.
  }

  ArrMD.push_back(
      CreateTypeMD(ClangType->castAsArrayTypeUnsafe()->getElementType(),
                   LLVMType->getArrayElementType(), CurInit));
  return llvm::MDNode::get(Ctx, ArrMD);
}

// Add the DTrans metadata for a type-info C++ type.
llvm::MDNode *
DTransInfoGenerator::AddTypeInfo(ArrayRef<llvm::Constant *> Fields) {
  // These are just represented as a literal structure with pointers all as i8*.
  // Fill this in so we don't confuse DTrans.
  llvm::SmallVector<llvm::Metadata *> MD;
  MD.push_back(llvm::MDString::get(Ctx, "L"));
  MD.push_back(llvm::ConstantAsMetadata::get(
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctx), Fields.size())));

  for (llvm::Constant *F : Fields) {
    llvm::Type *T = F->getType();

    if (T->isPointerTy()) {
      // The only pointer types that are seemingly emitted are i8*.
      MD.push_back(llvm::MDNode::get(
          Ctx, {llvm::ConstantAsMetadata::get(
                    llvm::Constant::getNullValue(llvm::Type::getInt8Ty(Ctx))),
                llvm::ConstantAsMetadata::get(
                    llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctx), 1))}));
    } else if (T->isIntegerTy()) {
      MD.push_back(llvm::MDNode::get(
          Ctx, {llvm::ConstantAsMetadata::get(llvm::Constant::getNullValue(T)),
                llvm::ConstantAsMetadata::get(
                    llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctx), 0))}));
    } else {
      // FIXME: Handle cases where other types can get here.
      llvm_unreachable("Unknown TypeInfo type");
    }
  }

  return llvm::MDNode::get(Ctx, MD);
}

// Add the information for a type's VTable.
llvm::MDNode *DTransInfoGenerator::AddVTable(const VTableLayout &Layout) {
  // The final element of the arrays is either the 'relative' value, so an
  // i32, or a generic pointer.
  llvm::Type *EltTy = CGM.getVTables().useRelativeLayout()
                          ? llvm::Type::getInt32Ty(Ctx)
                          : llvm::Type::getInt8Ty(Ctx);

  // Element is a i8*, or an i32.
  llvm::MDNode *Elt = llvm::MDNode::get(
      Ctx, {llvm::ConstantAsMetadata::get(llvm::Constant::getNullValue(EltTy)),
            llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(
                llvm::Type::getInt32Ty(Ctx),
                !CGM.getVTables().useRelativeLayout()))});

  // VTables are just a literal struct.
  llvm::SmallVector<llvm::Metadata *> StructMD;
  StructMD.push_back(llvm::MDString::get(Ctx, "L"));
  StructMD.push_back(llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(
      llvm::Type::getInt32Ty(Ctx), Layout.getNumVTables())));

  // Each is an array of elements: !{!"A", i32 NumElem, !Elt}
  for (unsigned I = 0, E = Layout.getNumVTables(); I < E; ++I)
    StructMD.push_back(llvm::MDNode::get(
        Ctx, {llvm::MDString::get(Ctx, "A"),
              llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(
                  llvm::Type::getInt32Ty(Ctx), Layout.getVTableSize(I))),
              Elt}));

  return llvm::MDNode::get(Ctx, StructMD);
}

} // namespace
#endif // INTEL_FEATURE_SW_DTRANS
