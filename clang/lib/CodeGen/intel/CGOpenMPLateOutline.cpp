#if INTEL_COLLAB                                           // -*- C++ -*-
//===--- CGOpenMPLateOutline.cpp - OpenMP Late-Outlining ------*- C++ -*---===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This contains code to emit OpenMP nodes as LLVM code.  It emits regions
// around OpenMP constructs instead of making calls to the OpenMP runtime.
//
//===----------------------------------------------------------------------===//

#include "CGOpenMPLateOutline.h"
#include "CGOpenMPRuntime.h"
#include "CGCleanup.h"
#include "clang/AST/Attr.h"
#include "clang/AST/StmtVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
using namespace clang;
using namespace CodeGen;
using namespace llvm::omp;

llvm::Value *OpenMPLateOutliner::emitOpenMPDefaultConstructor(const Expr *IPriv,
                                                              bool IsUDR) {
  CodeGenModule &CGM = CGF.CGM;
  // If generating device code, don't emit the routine if we are not inside
  // a target region, since it won't be used and will cause problems if
  // used from a later target region.
  if (CGF.getLangOpts().OpenMPIsDevice && !CGM.inTargetRegion())
    return llvm::ConstantPointerNull::get(CGF.VoidPtrTy);

  if (!IPriv)
    return llvm::ConstantPointerNull::get(CGF.VoidPtrTy);

  auto *Private = cast<VarDecl>(cast<DeclRefExpr>(IPriv)->getDecl());
  QualType Ty = Private->getType();

  SmallString<256> OutName;
  llvm::raw_svector_ostream Out(OutName);
  auto &Ctx = CGM.getContext();
  bool UseSingleArrayFuncs = CGF.getLangOpts().OpenMPUseSingleElemArrayFuncs;
  if (Ty->isArrayType() && (UseSingleArrayFuncs || IsUDR))
    Ty = Ctx.getBaseElementType(Ty).getNonReferenceType();
  CGM.getCXXABI().getMangleContext().mangleTypeName(Ty, Out);
  Out << ".omp.def_constr";

  if (llvm::Value *F = CGM.GetGlobalValue(OutName))
    return F;

  // Generate function that re-emits the declaration's initializer into the
  // threadprivate copy of the variable VD
  QualType PtrTy = Ctx.getPointerType(Ty);
  CodeGenFunction NewCGF(CGM);

  NewCGF.disableDebugInfo(); // No valid locations for this routine.

  FunctionArgList Args;
  ImplicitParamDecl Dst(CGM.getContext(), /*DC=*/nullptr, SourceLocation(),
                        /*Id=*/nullptr, PtrTy, ImplicitParamDecl::Other);
  Args.push_back(&Dst);

  auto &FI = CGM.getTypes().arrangeBuiltinFunctionDeclaration(PtrTy, Args);
  auto FTy = CGM.getTypes().GetFunctionType(FI);
  auto *Fn = CGM.CreateGlobalInitOrCleanUpFunction(FTy, OutName, FI);
  NewCGF.StartFunction(GlobalDecl(), PtrTy, Fn, FI, Args, SourceLocation());
  auto *Init = Private->getInit();
  if (Init && !NewCGF.isTrivialInitializer(Init)) {
    CodeGenFunction::RunCleanupsScope Scope(NewCGF);
    if (Init->getType()->isArrayType() != Ty->isArrayType()) {
      // Rebuild the constructor for the element type.
      if (auto Cleanups = dyn_cast<ExprWithCleanups>(Init))
        Init = Cleanups->getSubExpr();
      auto *CCE = cast<CXXConstructExpr>(Init);
      SmallVector<Expr *, 8> ConstructorArgs;
      for (auto I = CCE->arg_begin(), End = CCE->arg_end(); I != End; ++I)
        ConstructorArgs.push_back(const_cast<Expr *>(*I));

      Init = CXXConstructExpr::Create(
        Ctx, Ty, CCE->getLocation(), CCE->getConstructor(), CCE->isElidable(),
        ConstructorArgs, CCE->hadMultipleCandidates(),
        CCE->isListInitialization(), CCE->isStdInitListInitialization(),
        CCE->requiresZeroInitialization(), CCE->getConstructionKind(),
        CCE->getParenOrBraceRange());
    }
    LValue ArgLVal = NewCGF.EmitLoadOfPointerLValue(
        NewCGF.GetAddrOfLocalVar(&Dst), PtrTy->getAs<PointerType>());
    NewCGF.EmitAnyExprToMem(Init, ArgLVal.getAddress(NewCGF),
                            Ty.getQualifiers(), /*IsInitializer=*/true);
    NewCGF.Builder.CreateStore(ArgLVal.getPointer(NewCGF), NewCGF.ReturnValue);
  }
  NewCGF.FinishFunction();
  return Fn;
}

llvm::Value *
OpenMPLateOutliner::emitOpenMPDestructor(QualType Ty, bool IsUDR) {
  CodeGenModule &CGM = CGF.CGM;
  // If generating device code, don't emit the routine if we are not inside
  // a target region, since it won't be used and will cause problems if
  // used from a later target region.
  if (CGF.getLangOpts().OpenMPIsDevice && !CGM.inTargetRegion())
    return llvm::ConstantPointerNull::get(CGF.VoidPtrTy);

  SmallString<256> OutName;
  llvm::raw_svector_ostream Out(OutName);
  auto &Ctx = CGM.getContext();
  bool UseSingleArrayFuncs = CGF.getLangOpts().OpenMPUseSingleElemArrayFuncs;
  if (Ty->isArrayType() && (UseSingleArrayFuncs || IsUDR))
    Ty = Ctx.getBaseElementType(Ty).getNonReferenceType();
  CGM.getCXXABI().getMangleContext().mangleTypeName(Ty, Out);
  Out << ".omp.destr";

  if (llvm::Value *F = CGM.GetGlobalValue(OutName))
    return F;

  // Generate function that emits destructor call for the threadprivate copy
  // of the variable VD
  QualType PtrTy = Ctx.getPointerType(Ty);
  CodeGenFunction NewCGF(CGM);

  NewCGF.disableDebugInfo(); // No valid locations for this routine.

  FunctionArgList Args;
  ImplicitParamDecl Dst(CGM.getContext(), /*DC=*/nullptr, SourceLocation(),
                        /*Id=*/nullptr, PtrTy, ImplicitParamDecl::Other);
  Args.push_back(&Dst);

  auto &FI = CGM.getTypes().arrangeBuiltinFunctionDeclaration(
      CGM.getContext().VoidTy, Args);
  auto FTy = CGM.getTypes().GetFunctionType(FI);
  auto *Fn = CGM.CreateGlobalInitOrCleanUpFunction(FTy, OutName, FI);
  NewCGF.StartFunction(GlobalDecl(), CGM.getContext().VoidTy, Fn, FI, Args,
                    SourceLocation());
  if (Ty.isDestructedType() != QualType::DK_none) {
    CodeGenFunction::RunCleanupsScope Scope(NewCGF);
    LValue ArgLVal = NewCGF.EmitLoadOfPointerLValue(
                                                 NewCGF.GetAddrOfLocalVar(&Dst),
                                                 PtrTy->getAs<PointerType>());
    NewCGF.emitDestroy(ArgLVal.getAddress(NewCGF), Ty,
                       NewCGF.getDestroyer(Ty.isDestructedType()),
                       NewCGF.needsEHCleanup(Ty.isDestructedType()));
  }
  NewCGF.FinishFunction();
  return Fn;
}

llvm::Value *OpenMPLateOutliner::emitOpenMPCopyConstructor(const Expr *IPriv) {
  CodeGenModule &CGM = CGF.CGM;
  // If generating device code, don't emit the routine if we are not inside
  // a target region, since it won't be used and will cause problems if
  // used from a later target region.
  if (CGF.getLangOpts().OpenMPIsDevice && !CGM.inTargetRegion())
    return llvm::ConstantPointerNull::get(CGF.VoidPtrTy);

  if (!IPriv)
    return llvm::ConstantPointerNull::get(CGF.VoidPtrTy);

  auto *Private = cast<VarDecl>(cast<DeclRefExpr>(IPriv)->getDecl());

  auto &C = CGM.getContext();
  bool UseSingleArrayFuncs = CGF.getLangOpts().OpenMPUseSingleElemArrayFuncs;
  QualType Ty = Private->getType();
  QualType ElemType = Ty;
  if (Ty->isArrayType()) {
    ElemType = C.getBaseElementType(Ty).getNonReferenceType();
    if (UseSingleArrayFuncs)
      Ty = ElemType;
  }

  SmallString<256> OutName;
  llvm::raw_svector_ostream Out(OutName);
  CGM.getCXXABI().getMangleContext().mangleTypeName(Ty, Out);
  Out << ".omp.copy_constr";

  if (llvm::Value *F = CGM.GetGlobalValue(OutName))
    return F;

  // Note that we should be able to optimize this to return the cctor directly
  // in cases where this is only a simple call.

  // Generate a copy constructor wrapper for the type of the firstprivate
  // variable.  Note this must also handle array types. This is similar to
  // the code in emitOpenMPDefaultConstructor but the Init passed here
  // (which contains references to early outlining variables that do not exist)
  // must be processed to use the source object address passed in as a
  // parameter. Since we need to create new AST objects in the generated
  // routine we need to create a FunctionDecl to act as the DeclContext.

  IdentifierInfo *II = &CGM.getContext().Idents.get(OutName);
  FunctionProtoType::ExtProtoInfo EPI;
  QualType FunctionTy = C.getFunctionType(C.VoidTy, llvm::None, EPI);
  FunctionDecl *FD = FunctionDecl::Create(
      C, C.getTranslationUnitDecl(), SourceLocation(), SourceLocation(), II,
      FunctionTy, C.getTrivialTypeSourceInfo(FunctionTy),
      StorageClass::SC_Static);

  QualType ObjPtrTy = C.getPointerType(Ty);

  CodeGenFunction NewCGF(CGM);

  NewCGF.disableDebugInfo(); // No valid locations for this routine.

  FunctionArgList Args;
  ImplicitParamDecl DstDecl(C, FD, SourceLocation(), nullptr, ObjPtrTy,
                            ImplicitParamDecl::Other);
  Args.push_back(&DstDecl);
  ImplicitParamDecl SrcDecl(C, FD, SourceLocation(), nullptr, ObjPtrTy,
                            ImplicitParamDecl::Other);
  Args.push_back(&SrcDecl);

  const CGFunctionInfo &FI =
      CGM.getTypes().arrangeBuiltinFunctionDeclaration(C.VoidTy, Args);

  llvm::FunctionType *LTy = CGM.getTypes().GetFunctionType(FI);

  llvm::Function *Fn = llvm::Function::Create(
      LTy, llvm::GlobalValue::InternalLinkage, OutName, &CGM.getModule());

  CGM.SetInternalFunctionAttributes(GlobalDecl(), Fn, FI);

  NewCGF.StartFunction(FD, C.VoidTy, Fn, FI, Args);
  auto *Init = Private->getInit();
  if (Init && !NewCGF.isTrivialInitializer(Init)) {
    CodeGenFunction::RunCleanupsScope Scope(NewCGF);
    // Initializer might involve cleanups, so skip to constructor.
    if (auto Cleanups = dyn_cast<ExprWithCleanups>(Init))
      Init = Cleanups->getSubExpr();
    auto *CCE = cast<CXXConstructExpr>(Init);
    DeclRefExpr SrcExpr(C, &SrcDecl,
                        /*RefersToEnclosingVariableOrCapture=*/false,
                        ObjPtrTy, VK_LValue, SourceLocation());
    ImplicitCastExpr CastExpr(ImplicitCastExpr::OnStack,
                              C.getPointerType(ElemType), CK_BitCast, &SrcExpr,
                              VK_PRValue, FPOptionsOverride());
    UnaryOperator *SRC = UnaryOperator::Create(
        C, &CastExpr, UO_Deref, ElemType, VK_LValue, OK_Ordinary,
        SourceLocation(), /*CanOverflow=*/false, FPOptionsOverride());

    QualType CTy = ElemType;
    CTy.addConst();
    ImplicitCastExpr NoOpCast(ImplicitCastExpr::OnStack, CTy, CK_NoOp, SRC,
                              VK_LValue, FPOptionsOverride());

    SmallVector<Expr *, 8> ConstructorArgs;
    ConstructorArgs.push_back(&NoOpCast);
    // Add possible default arguments, which start with the second arg.
    for (auto I = CCE->arg_begin() + 1, End = CCE->arg_end(); I != End; ++I)
      ConstructorArgs.push_back(const_cast<Expr *>(*I));

    CXXConstructExpr *RebuiltCCE = CXXConstructExpr::Create(
        C, Ty, CCE->getLocation(), CCE->getConstructor(), CCE->isElidable(),
        ConstructorArgs, CCE->hadMultipleCandidates(),
        CCE->isListInitialization(), CCE->isStdInitListInitialization(),
        CCE->requiresZeroInitialization(), CCE->getConstructionKind(),
        CCE->getParenOrBraceRange());

    LValue ArgLVal = NewCGF.EmitLoadOfPointerLValue(
        NewCGF.GetAddrOfLocalVar(&DstDecl), ObjPtrTy->getAs<PointerType>());
    NewCGF.EmitAnyExprToMem(RebuiltCCE, ArgLVal.getAddress(NewCGF),
                            Ty.getQualifiers(), /*IsInitializer=*/true);
  }
  NewCGF.FinishFunction();

  return Fn;
}

llvm::Value *OpenMPLateOutliner::emitOpenMPCopyAssign(QualType Ty,
    const Expr *SrcExpr, const Expr *DstExpr, const Expr *AssignOp) {
  CodeGenModule &CGM = CGF.CGM;
  // If generating device code, don't emit the routine if we are not inside
  // a target region, since it won't be used and will cause problems if
  // used from a later target region.
  if (CGF.getLangOpts().OpenMPIsDevice && !CGM.inTargetRegion())
    return llvm::ConstantPointerNull::get(CGF.VoidPtrTy);

  auto &C = CGM.getContext();
  bool UseSingleArrayFuncs = CGF.getLangOpts().OpenMPUseSingleElemArrayFuncs;
  QualType ElemType = Ty;
  if (Ty->isArrayType()) {
    ElemType = C.getBaseElementType(Ty).getNonReferenceType();
    if (UseSingleArrayFuncs)
      Ty = ElemType;
  }

  SmallString<256> OutName;
  llvm::raw_svector_ostream Out(OutName);
  CGM.getCXXABI().getMangleContext().mangleTypeName(Ty, Out);
  Out << ".omp.copy_assign";

  if (llvm::Value *F = CGM.GetGlobalValue(OutName))
    return F;

  IdentifierInfo *II = &CGM.getContext().Idents.get(OutName);
  FunctionProtoType::ExtProtoInfo EPI;
  QualType FunctionTy = C.getFunctionType(C.VoidTy, llvm::None, EPI);
  FunctionDecl *FD = FunctionDecl::Create(
      C, C.getTranslationUnitDecl(), SourceLocation(), SourceLocation(), II,
      FunctionTy, C.getTrivialTypeSourceInfo(FunctionTy),
      StorageClass::SC_Static);

  QualType ObjPtrTy = C.getPointerType(Ty);

  CodeGenFunction NewCGF(CGM);

  NewCGF.disableDebugInfo(); // No valid locations for this routine.

  FunctionArgList Args;
  ImplicitParamDecl DstDecl(C, FD, SourceLocation(), nullptr, ObjPtrTy,
                            ImplicitParamDecl::Other);
  Args.push_back(&DstDecl);
  ImplicitParamDecl SrcDecl(C, FD, SourceLocation(), nullptr, ObjPtrTy,
                            ImplicitParamDecl::Other);
  Args.push_back(&SrcDecl);

  const CGFunctionInfo &FI =
      CGM.getTypes().arrangeBuiltinFunctionDeclaration(C.VoidTy, Args);

  llvm::FunctionType *LTy = CGM.getTypes().GetFunctionType(FI);

  llvm::Function *Fn = llvm::Function::Create(
      LTy, llvm::GlobalValue::InternalLinkage, OutName, &CGM.getModule());

  CGM.SetInternalFunctionAttributes(GlobalDecl(), Fn, FI);

  NewCGF.StartFunction(FD, C.VoidTy, Fn, FI, Args);

  auto DestAddr = NewCGF.EmitLoadOfPointerLValue(
                             NewCGF.GetAddrOfLocalVar(&DstDecl),
                             ObjPtrTy->getAs<PointerType>()).getAddress(NewCGF);

  auto SrcAddr = NewCGF.EmitLoadOfPointerLValue(
                            NewCGF.GetAddrOfLocalVar(&SrcDecl),
                            ObjPtrTy->getAs<PointerType>()).getAddress(NewCGF);

  auto *SrcVD = cast<VarDecl>(cast<DeclRefExpr>(SrcExpr)->getDecl());
  auto *DestVD = cast<VarDecl>(cast<DeclRefExpr>(DstExpr)->getDecl());

  NewCGF.EmitOMPCopy(Ty, DestAddr, SrcAddr, DestVD, SrcVD, AssignOp);

  NewCGF.FinishFunction();
  return Fn;
}

/// Returns the base expression for an array section or an array subscript
/// used where an array section is expected.  Optionally (if AS is not
/// a nullptr) fills in the array section data for each dimension.
const Expr *OpenMPLateOutliner::getArraySectionBase(const Expr *E,
                                                    CodeGenFunction *CGF,
                                                    ArraySectionTy *AS) {
  const Expr *Base = E->IgnoreParenImpCasts();
  while (const auto *TempOASE = dyn_cast<OMPArraySectionExpr>(Base)) {
    if (AS)
      AS->insert(AS->begin(), emitArraySectionData(Base, *CGF));
    Base = TempOASE->getBase()->IgnoreParenImpCasts();
  }
  while (const auto *TempASE = dyn_cast<ArraySubscriptExpr>(Base)) {
    if (AS)
      AS->insert(AS->begin(), emitArraySectionData(Base, *CGF));
    Base = TempASE->getBase()->IgnoreParenImpCasts();
  }
  return Base;
}

OpenMPLateOutliner::ArraySectionDataTy
OpenMPLateOutliner::emitArraySectionData(const Expr *E, CodeGenFunction &CGF) {
  ArraySectionDataTy Data;
  auto &C = CGF.getContext();

  if (auto *ASE = dyn_cast<ArraySubscriptExpr>(E)) {
    auto *Index = ASE->getIdx();
    Data.LowerBound =
        CGF.EmitScalarConversion(CGF.EmitScalarExpr(Index), Index->getType(),
                                 C.getSizeType(), Index->getExprLoc());
    Data.Length = llvm::ConstantInt::get(CGF.SizeTy, /*V=*/1);
    Data.Stride = llvm::ConstantInt::get(CGF.SizeTy, /*V=*/1);
    return Data;
  }

  auto *OASE = cast<OMPArraySectionExpr>(E);
  if (auto *LowerBound = OASE->getLowerBound()) {
    Data.LowerBound = CGF.EmitScalarConversion(
        CGF.EmitScalarExpr(LowerBound), LowerBound->getType(), C.getSizeType(),
        LowerBound->getExprLoc());
  } else
    Data.LowerBound = llvm::ConstantInt::getNullValue(CGF.SizeTy);
  QualType BaseTy = OMPArraySectionExpr::getBaseOriginalType(
      OASE->getBase()->IgnoreParenImpCasts());
  if (auto *Length = OASE->getLength()) {
    Data.Length =
        CGF.EmitScalarConversion(CGF.EmitScalarExpr(Length), Length->getType(),
                                 C.getSizeType(), Length->getExprLoc());
  } else {
    Optional<llvm::APSInt> ConstLength;
    if (auto *VAT = C.getAsVariableArrayType(BaseTy)) {
      Length = VAT->getSizeExpr();
      ConstLength = Length->getIntegerConstantExpr(C);
      if (ConstLength)
        Length = nullptr;
    } else {
      auto *CAT = C.getAsConstantArrayType(BaseTy);
      ConstLength = llvm::APSInt{CAT->getSize()};
    }
    llvm::Value *LengthVal;
    if (Length) {
      LengthVal = CGF.EmitScalarConversion(CGF.EmitScalarExpr(Length),
                                           Length->getType(), C.getSizeType(),
                                           Length->getExprLoc());
    } else {
      LengthVal = llvm::ConstantInt::get(CGF.SizeTy, ConstLength->getExtValue());
    }
    Data.Length = CGF.Builder.CreateSub(LengthVal, Data.LowerBound);
  }
  Data.Stride = llvm::ConstantInt::get(CGF.SizeTy, /*V=*/1);
  return Data;
}

/// Emit an address of the base of OMPArraySectionExpr and fills data for
/// array sections.
Address OpenMPLateOutliner::emitOMPArraySectionExpr(const Expr *E,
                                                    ArraySectionTy *AS) {
  const Expr *Base = getArraySectionBase(E, &CGF, AS);
  QualType BaseTy = Base->getType();
  Address BaseAddr = CGF.EmitLValue(Base).getAddress(CGF);
  if (BaseTy->isVariablyModifiedType()) {
    for (auto &ASD : *AS) {
      if (const ArrayType *AT = BaseTy->getAsArrayTypeUnsafe()) {
        BaseTy = AT->getElementType();
        llvm::Value *Size = nullptr;
        if (auto *VAT = dyn_cast<VariableArrayType>(AT)) {
          Size = CGF.EmitScalarConversion(
              CGF.EmitScalarExpr(VAT->getSizeExpr()),
              VAT->getSizeExpr()->getType(), CGF.getContext().getSizeType(),
              SourceLocation());
        } else if (auto *CAT = dyn_cast<ConstantArrayType>(AT))
          Size = llvm::ConstantInt::get(CGF.SizeTy, CAT->getSize());
        else
          Size = llvm::ConstantPointerNull::get(CGF.VoidPtrTy);
        ASD.VLASize = Size;
      } else {
        assert((BaseTy->isPointerType()));
        BaseTy = BaseTy->getPointeeType();
        ASD.VLASize = llvm::ConstantPointerNull::get(CGF.VoidPtrTy);
      }
    }
  }
  return BaseAddr;
}

void OpenMPLateOutliner::addArg(StringRef Str) { BundleString = Str; }

void OpenMPLateOutliner::addArg(llvm::Value *V, bool Handled, bool IsTyped,
                                llvm::Type *ElementType, llvm::Value *ZeroValue,
                                llvm::Value *NumElements) {
  BundleValues.push_back(V);
  if (Handled)
    HandledValues.insert(V);
  if (IsTyped) {
    BundleValues.push_back(
        ZeroValue ? ZeroValue : llvm::Constant::getNullValue(ElementType));
    if (NumElements)
      BundleValues.push_back(NumElements);
  }
}

void OpenMPLateOutliner::addNoElementTypedArg(llvm::Value *V,
                                              llvm::Type *ElementType,
                                              bool Handled) {
  addArg(V, Handled, UseTypedClauses, ElementType);
}

void OpenMPLateOutliner::addSingleElementTypedArg(llvm::Value *V,
                                                  llvm::Type *ElementType,
                                                  bool Handled) {
  addArg(V, Handled, UseTypedClauses, ElementType, /*ZeroValue=*/nullptr,
         CGF.Builder.getInt32(1));
}

void OpenMPLateOutliner::addArg(const Expr *E, bool IsRef, bool IsTyped,
                                bool NeedsTypedElements,
                                llvm::Type *ElementType,
                                bool ArraySecUsesBase) {
  if (isa<ArraySubscriptExpr>(E->IgnoreParenImpCasts()) ||
      E->getType()->isSpecificPlaceholderType(BuiltinType::OMPArraySection)) {

    // Array sections can be emitted in two typed forms.
    // If ArraySecUsesBase is true, then it is emitted as:
    // section-base, type specifier, number of elements, offset in elements.
    // If ArraySecUsesBase is false, then it is emiitted as:
    // address of first element of section, type specifier, number of elements.

    ArraySectionTy AS;
    llvm::Value *BaseAddr = nullptr;
    if (ArraySecUsesBase) {
      BaseAddr = emitOMPArraySectionExpr(E, &AS).getPointer();
      if (IsRef) {
        auto *LI = dyn_cast<llvm::LoadInst>(BaseAddr);
        assert(LI && "expected load instruction for reference type");
        BaseAddr = LI->getPointerOperand();
      }
      addArg(BaseAddr, /*Handled=*/true);
    }

    LValue LowerBound;
    if (IsTyped) {
      if (auto *AS = dyn_cast<OMPArraySectionExpr>(E->IgnoreParenImpCasts()))
        LowerBound = CGF.EmitOMPArraySectionExpr(AS, /*IsLowerBound=*/true);
      else
        LowerBound = CGF.EmitLValue(E);
    }

    if (!ArraySecUsesBase)
      addArg(LowerBound.getPointer(CGF), /*Handled=*/true);

    if (IsTyped) {
      const Expr *Base = getArraySectionBase(E);
      QualType BaseT(Base->getType()->getPointeeOrArrayElementType(), 0);
      if (BaseT->isArrayType())
        BaseT = QualType(BaseT->getPointeeOrArrayElementType(), 0);
      CharUnits ElementSize = CGF.getContext().getTypeSizeInChars(BaseT);
      llvm::Value *Size = CGF.CGM.getSize(ElementSize);;

      addArg(llvm::Constant::getNullValue(
          ElementType ? ElementType : CGF.CGM.getTypes().ConvertType(BaseT)));

      llvm::Value *BaseCast = nullptr;
      if (ArraySecUsesBase) {
        // If the array section variable is a pointer or reference, it contains
        // the address of the first element and it differs from the address
        // used in the first Arg. Load it here.
        if (Base->getType()->isPointerType())
          BaseAddr = CGF.EmitPointerWithAlignment(Base).getPointer();
        else if (IsRef)
          BaseAddr = CGF.EmitLValue(Base).getPointer(CGF);
        BaseCast =
            CGF.Builder.CreatePtrToInt(BaseAddr, CGF.PtrDiffTy, "sec.base.cast");
      }

      llvm::Value *NumElements;
      llvm::Value *OffsetInElements;

      if (auto *AS = dyn_cast<OMPArraySectionExpr>(E->IgnoreParenImpCasts())) {
        llvm::Value *LowerCast = CGF.Builder.CreatePtrToInt(
            LowerBound.getPointer(CGF), CGF.PtrDiffTy, "sec.lower.cast");

        LValue UpperBound = CGF.EmitOMPArraySectionExpr(AS, /*IsLowerBound=*/false);
        llvm::Value *UpperCast = CGF.Builder.CreatePtrToInt(
            UpperBound.getPointer(CGF), CGF.PtrDiffTy, "sec.upper.cast");

        llvm::Value *NumElementsInChars = CGF.Builder.CreateSub(UpperCast, LowerCast);
        NumElements = ElementSize.isOne() ? NumElementsInChars :
              CGF.Builder.CreateExactSDiv(NumElementsInChars, Size);
        NumElements = CGF.Builder.CreateAdd(
            NumElements, llvm::ConstantInt::get(CGF.PtrDiffTy, /*V=*/1),
            "sec.number_of_elements");

        if (ArraySecUsesBase) {
          llvm::Value *OffsetInChars =
              CGF.Builder.CreateSub(LowerCast, BaseCast);
          OffsetInElements =
              ElementSize.isOne()
                  ? OffsetInChars
                  : CGF.Builder.CreateExactSDiv(OffsetInChars, Size,
                                                "sec.offset_in_elements");
        }
      } else {
        assert(isa<ArraySubscriptExpr>(E->IgnoreParenImpCasts()) &&
               "expected subscript expression");

        NumElements = llvm::ConstantInt::get(CGF.PtrDiffTy, /*V=*/1);
        if (ArraySecUsesBase) {
          llvm::Value *LowerCast = CGF.Builder.CreatePtrToInt(
              LowerBound.getPointer(CGF), CGF.PtrDiffTy, "sec.lower.cast");
          llvm::Value *OffsetInChars =
              CGF.Builder.CreateSub(LowerCast, BaseCast);
          OffsetInElements =
              ElementSize.isOne()
                  ? OffsetInChars
                  : CGF.Builder.CreateExactSDiv(OffsetInChars, Size,
                                                "sec.offset_in_elements");
        }
      }
      addArg(NumElements);
      if (ArraySecUsesBase)
        addArg(OffsetInElements);
    } else {
      addArg(llvm::ConstantInt::get(CGF.SizeTy, AS.size()));
      for (auto &V : AS) {
        assert(V.LowerBound);
        addArg(V.LowerBound);
        assert(V.Length);
        addArg(V.Length);
        assert(V.Stride);
        addArg(V.Stride);
      }
    }
  } else {
    assert(E->isGLValue());
    if (const auto *DRE = dyn_cast<DeclRefExpr>(E)) {
      const VarDecl *VD = cast<VarDecl>(DRE->getDecl());
      if (VD && CGF.isMappedRefTemp(VD))
        IsRef = true;
    }
    LValue LV = CGF.EmitLValue(E);
    Address Addr = LV.getAddress(CGF);
    llvm::Value *V = LV.getPointer(CGF);
    if (IsRef) {
      auto *LI = dyn_cast<llvm::LoadInst>(V);
      assert(LI && "expected load instruction for reference type");
      V = LI->getPointerOperand();
    }
    llvm::Value *Elements = nullptr;
    llvm::Value *ZeroValue = nullptr;
    if (IsTyped) {
      if (const auto *AT = dyn_cast<ArrayType>(E->getType())) {
        QualType BaseT;
        Elements = CGF.emitArrayLength(AT, BaseT, Addr);
        ZeroValue = llvm::Constant::getNullValue(CGF.CGM.getTypes().ConvertType(BaseT));
      } else {
        if (NeedsTypedElements)
          Elements = CGF.Builder.getInt32(1);
      }
    }
    addArg(V, /*Handled=*/true, IsTyped,
           ElementType ? ElementType : Addr.getElementType(), ZeroValue,
           Elements);
  }
}

void OpenMPLateOutliner::addTypedArg(const Expr *E, bool IsRef,
                                     bool NeedsTypedElements) {
  addArg(E, IsRef, UseTypedClauses, NeedsTypedElements);
}

/// Verify current if-clause applies to current directive, which can be
/// an individual directive or part of a set of directives connected
/// together. An if-clause applies if:
/// - it is implicit, or
/// - it has no name modifier associated with it, or
/// - its name modifier matches current directive (DKind)
///
/// An example: the distribute parallel for simd directive is broken down
/// into distribute paralle for, and, simd, both of which will come through
/// this function from getApplicableDirectives. An if-clause specified with
/// distribute parallel for simd can specify no name modifier, or parallel,
/// or simd, or both.
bool OpenMPLateOutliner::checkIfModifier(OpenMPDirectiveKind DKind,
                                         const OMPIfClause *IC) {

  // If no if-clause(s) with original directive, then this if-clause is
  // implicit.
  if (!Directive.hasClausesOfKind<OMPIfClause>())
    return true;
  OpenMPDirectiveKind NameModifier = IC ? IC->getNameModifier() : OMPD_unknown;

  // If no name modifier, or name modifier matches current directive, or
  // name modifier is parallel and the current directive is another form
  // of the parallel directive, if-clause should be applied.
  if (NameModifier == OMPD_unknown || NameModifier == DKind ||
     (NameModifier == OMPD_parallel && (DKind == OMPD_parallel_for ||
     DKind == OMPD_parallel_sections || DKind == OMPD_distribute_parallel_for)))
    return true;

  // No match for current if-clause/modifier and current directive. Should not
  // be applied.
  return false;
}

void OpenMPLateOutliner::getApplicableDirectives(
    OpenMPClauseKind CK, ImplicitClauseKind ICK,
    SmallVector<DirectiveIntrinsicSet *, 4> &Dirs) {

  // Place implicit linears on SIMD only.
  if (ICK == ICK_linear) {
    Dirs.push_back(&Directives.back());
    return;
  }

  // OMPC_unknown is used in cases where there is no real clause kind, or
  // for implicit clauses that need to be skipped on simd when simd is
  // part of another loop directive.

  // This is likely to become complicated but for now if there is more than
  // one directive, place the clause on each if the clause is allowed there.

  bool LoopSeen = false;
  for (auto &D : Directives) {
    // Implicit clauses (OMPC_unknown) should be left off the simd part of
    // a multiple directive loop set.
    if (LoopSeen && D.DKind == OMPD_simd && CK == OMPC_unknown)
      return;

    if (isOpenMPLoopDirective(D.DKind))
      LoopSeen = true;

    // Implicit clauses are only added when applicable, so no further checks are
    // needed.
    if (CK == OMPC_unknown) {
      Dirs.push_back(&D);
      continue;
    }
    if (isAllowedClauseForDirectiveFull(D.DKind, CK, ICK))
      Dirs.push_back(&D);
  }
}

void OpenMPLateOutliner::startDirectiveIntrinsicSet(StringRef Begin,
                                                    StringRef End,
                                                    OpenMPDirectiveKind K) {
  assert(BundleValues.empty());
  DirectiveIntrinsicSet D(End, K);
  llvm::OperandBundleDef B(std::string(Begin), BundleValues);
  D.OpBundles.push_back(B);
  Directives.push_back(D);
}

void OpenMPLateOutliner::emitDirective(DirectiveIntrinsicSet &D,
                                       StringRef Name) {
  assert(BundleValues.empty());
  llvm::OperandBundleDef B(std::string(Name), BundleValues);
  D.OpBundles.push_back(B);
  clearBundleTemps();
}

void OpenMPLateOutliner::emitClause(OpenMPClauseKind CK,
                                    ImplicitClauseKind ICK) {
  SmallVector<DirectiveIntrinsicSet *, 4> DRefs;
  getApplicableDirectives(CK, ICK, DRefs);
  for (auto *D : DRefs) {
    llvm::OperandBundleDef B(std::string(BundleString), BundleValues);
    D->OpBundles.push_back(B);
  }
  clearBundleTemps();
}

void OpenMPLateOutliner::emitImplicit(llvm::Value *V, llvm::Type *ElementType,
                                      ImplicitClauseKind K, bool Handled) {

  ClauseEmissionHelper CEH(*this, OMPC_unknown);
  ClauseStringBuilder &CSB = CEH.getBuilder();

  switch (K) {
  case ICK_private:
  case ICK_linear_private:
    CSB.add("QUAL.OMP.PRIVATE");
    break;
  case ICK_firstprivate:
    CSB.add("QUAL.OMP.FIRSTPRIVATE");
    break;
  case ICK_lastprivate:
  case ICK_linear_lastprivate:
    CSB.add("QUAL.OMP.LASTPRIVATE");
    break;
  case ICK_shared:
    CSB.add("QUAL.OMP.SHARED");
    break;
  default:
    llvm_unreachable("Clause not allowed");
  }
  if (UseTypedClauses)
    CSB.setTyped();
  addArg(CSB.getString());
  addSingleElementTypedArg(V, ElementType, Handled);
}

void OpenMPLateOutliner::emitImplicit(Expr *E, ImplicitClauseKind K) {
  if (K == ICK_linear || K == ICK_linear_private ||
      K == ICK_linear_lastprivate) {
    ClauseEmissionHelper CEH(*this, OMPC_unknown);
    ClauseStringBuilder &CSB = CEH.getBuilder();
    CSB.add("QUAL.OMP.LINEAR");
    CSB.setIV();
    if (UseTypedClauses)
      CSB.setTyped();
    CEH.setImplicitClause(ICK_linear);
    addArg(CSB.getString());
    addTypedArg(E);
    auto *LD = cast<OMPLoopDirective>(&Directive);
    addArg(emitSpecialSIMDExpression(LD->getLateOutlineLinearCounterStep()));
    // Plain SIMD doesn't use the private/lastprivate clause but the
    // implicit clause kind is used to determine if the increment is emitted.
    if (CurrentDirectiveKind == OMPD_simd)
      return;
  }

  ClauseEmissionHelper CEH(*this, OMPC_unknown);
  ClauseStringBuilder &CSB = CEH.getBuilder();

  switch (K) {
  case ICK_private:
    CEH.setClauseKind(OMPC_private);
    LLVM_FALLTHROUGH;
  case ICK_linear_private:
    CSB.add("QUAL.OMP.PRIVATE");
    break;
  case ICK_firstprivate:
    CSB.add("QUAL.OMP.FIRSTPRIVATE");
    break;
  case ICK_lastprivate:
    CEH.setClauseKind(OMPC_lastprivate);
    LLVM_FALLTHROUGH;
  case ICK_linear_lastprivate:
    CSB.add("QUAL.OMP.LASTPRIVATE");
    break;
  case ICK_normalized_iv:
    CSB.add("QUAL.OMP.NORMALIZED.IV");
    break;
  case ICK_normalized_ub:
    CSB.add("QUAL.OMP.NORMALIZED.UB");
    break;
  case ICK_livein:
    CSB.add("QUAL.OMP.LIVEIN");
    break;
  case ICK_shared:
    CSB.add("QUAL.OMP.SHARED");
    break;
  default:
    llvm_unreachable("Clause not allowed");
  }
  if (UseTypedClauses)
    CSB.setTyped();
  bool IsRef = false;
  if (K == ICK_shared) {
    const VarDecl *VD = getExplicitVarDecl(E);
    assert(VD && "expected VarDecl in implicit clause");
    IsRef = !isa<OMPCapturedExprDecl>(VD) && VD->getType()->isReferenceType();
    if (IsRef)
      CSB.setByRef();
  }
  addArg(CSB.getString());
  bool NeedsTypedElements =
      K == ICK_normalized_iv || K == ICK_normalized_ub ? false : true;
  addTypedArg(E, IsRef, NeedsTypedElements);
}

void OpenMPLateOutliner::emitImplicit(const VarDecl *VD, ImplicitClauseKind K) {
  // ICK_unknown is used when we do not want a variable to appear in any
  // clause list, so just return when we see it.
  if (K == ICK_unknown)
    return;

#if INTEL_CUSTOMIZATION
  if ((K == ICK_normalized_iv || K == ICK_normalized_ub) &&
      CGF.useUncollapsedLoop(cast<OMPLoopDirective>(Directive))) {
    // Add multiple IV/UB variables to a bundle.  When the first is
    // encountered add them all and change the kind to prevent further
    // processing.
    CodeGenFunction::CGCapturedStmtRAII SaveCSI(CGF, nullptr);
    ClauseEmissionHelper CEH(*this, OMPC_unknown);
    ClauseStringBuilder &CSB = CEH.getBuilder();
    if (K == ICK_normalized_iv)
      CSB.add("QUAL.OMP.NORMALIZED.IV");
    else
      CSB.add("QUAL.OMP.NORMALIZED.UB");
    if (UseTypedClauses)
      CSB.setTyped();
    addArg(CSB.getString());
    for (auto &A : ImplicitMap) {
      if (A.second == K) {
        DeclRefExpr DRE(CGF.CGM.getContext(), const_cast<VarDecl *>(A.first),
                        /*RefersToEnclosingVariableOrCapture=*/false,
                        A.first->getType().getNonReferenceType(), VK_LValue,
                        SourceLocation());
        addTypedArg(&DRE, /*IsRef=*/false, /*NeedsTypedElements=*/false);
        A.second = ICK_unknown;
      }
    }
    return;
  }
#endif // INTEL_CUSTOMIZATION

  bool VarDefinedWithAllocator =
      VD->getCanonicalDecl()->hasAttr<OMPAllocateDeclAttr>() &&
      VarDefs.find(VD) != VarDefs.end();
  if (!OMPLateOutlineLexicalScope::isCapturedVar(CGF, VD) &&
      !VarDefinedWithAllocator) {
    // We don't want this DeclRefExpr to generate entries in the Def/Ref
    // lists, so temporarily save and null the CapturedStmtInfo.
    CodeGenFunction::CGCapturedStmtRAII SaveCSI(CGF, nullptr);

    DeclRefExpr DRE(CGF.CGM.getContext(), const_cast<VarDecl *>(VD),
                    /*RefersToEnclosingVariableOrCapture=*/false,
                    VD->getType().getNonReferenceType(), VK_LValue,
                    SourceLocation());
    emitImplicit(&DRE, K);
  }
}

/// Returns true if this variable is specified implicitly and also
/// should not be propagated to outer regions.
bool OpenMPLateOutliner::isIgnoredImplicit(const VarDecl *V) {
  if (!isImplicit(V))
    return false;
  return ImplicitMap[V] == ICK_normalized_iv;
}

bool OpenMPLateOutliner::isImplicit(const VarDecl *V) {
  return ImplicitMap.find(V) != ImplicitMap.end();
}

/// There are special rules to handle clauses on combined/composite constructs
/// as specified in OpenMP 5.2 17.2. Given a construct that is a piece of the
/// full directive, return true if the clause should be added explicitly there.
/// This is a wrapper around the general isAllowedClauseForDirective for cases
/// that require seeing the full directive to decide.
bool OpenMPLateOutliner::isAllowedClauseForDirectiveFull(
    OpenMPDirectiveKind DKind, OpenMPClauseKind CK, ImplicitClauseKind ICK) {
  if (CK == OMPC_reduction) {
    if (CodeGenFunction::requiresImplicitTaskgroup(Directive)) {
      // Processing a directive with an implicit taskgroup, implement special
      // rules.
      if (DKind == OMPD_taskgroup && ICK == ICK_reduction)
        return true;
      if (DKind == OMPD_taskloop && ICK == ICK_inreduction)
        return true;
      if (ICK == ICK_inreduction)
        return false;
    }
    if (DKind == OMPD_target) {
      // Prevent reduction on target which is being allowed by
      // isAllowedClauseForDirective.
      return false;
    }
  } else if (CK == OMPC_firstprivate) {
    if (CK == OMPC_firstprivate && DKind == OMPD_teams &&
        isOpenMPDistributeDirective(Directive.getDirectiveKind())) {
      // The effect of the firstprivate clause is as if it is applied to one or
      // more leaf constructs as follows:
      //   To the distribute construct if it is among the constituent
      //   constructs; To the teams construct if it is among the constituent
      //   constructs and the distribute construct is not.
      // So if this is 'teams' within a distribute construct, don't place the
      // clause.
      return false;
    }
    if (DKind == OMPD_dispatch) {
      // The 'firstprivate' clause will appear only on the implicit task.
      return false;
    }
  } else if (CK == OMPC_if) {
    if (isAllowedClauseForDirective(DKind, CK, CGF.getLangOpts().OpenMP)) {
      const OMPIfClause *IC = dyn_cast_or_null<OMPIfClause>(CurrentClause);
      if (checkIfModifier(DKind, IC))
        return true;
    }
    return false;
  }
  return isAllowedClauseForDirective(DKind, CK, CGF.getLangOpts().OpenMP);
}

/// Returns true if an explicit clause was added to the Directive for
/// the variable V that is compatible for the DKind. This is used to
/// prevent the addition of implicit clauses where an explicit clause
/// applies, but still allows implicit clauses on parts of combined
/// directives.
bool OpenMPLateOutliner::isExplicitForDirective(const VarDecl *V,
                                                OpenMPDirectiveKind DKind) {
  const auto &It = ExplicitRefs.find(V);
  if (It == ExplicitRefs.end())
    return false;
  // Only need to check the actual clauses if this is a combined directive.
  if (DKind == Directive.getDirectiveKind())
    return true;
  const auto &ExplicitKinds = (*It).second;
  const auto CIt = std::find_if(
      ExplicitKinds.begin(), ExplicitKinds.end(),
      [this, DKind](OpenMPClauseKind CK) {
        return isAllowedClauseForDirectiveFull(DKind, CK, ICK_unknown);
      });
  return CIt != ExplicitKinds.end();
}

bool OpenMPLateOutliner::alreadyHandled(llvm::Value *V) {
  return HandledValues.find(V) != HandledValues.end();
}

void OpenMPLateOutliner::addImplicitClauses() {
  if (!isOpenMPLoopDirective(CurrentDirectiveKind) &&
      !isOpenMPParallelDirective(CurrentDirectiveKind) &&
      CurrentDirectiveKind != OMPD_scope &&
      CurrentDirectiveKind != OMPD_task && CurrentDirectiveKind != OMPD_loop &&
      CurrentDirectiveKind != OMPD_target && CurrentDirectiveKind != OMPD_teams)
    return;

  for (const auto *VD : VarRefs) {
    if (VD->getName().empty()) {
      // Avoid trying to create a clause for an unnamed catch parameter.
      continue;
    }
    if (isExplicitForDirective(VD, CurrentDirectiveKind))
      continue;
    if (isImplicit(VD)) {
      emitImplicit(VD, ImplicitMap[VD]);
      continue;
    }
    if (DependIteratorVars.find(VD) != DependIteratorVars.end()) {
      // Do not create implicit clauses for iterator vars.
      // These variables generate temps and are handled with Values.
      assert(VD->isImplicit() && "expect implicit variabe");
      continue;
    }
    if (VarDefs.find(VD) != VarDefs.end()) {
      // Defined in the region
      if (!VD->getType()->isConstantSizeType()) {
        // An alloca inserted inside the region cannot be used on a clause.
        continue;
      } else if (VD->getStorageDuration() == SD_Static) {
        if (CurrentDirectiveKind == OMPD_loop ||
            isAllowedClauseForDirective(CurrentDirectiveKind, OMPC_shared,
                                        CGF.getLangOpts().OpenMP))
          emitImplicit(VD, ICK_shared);
      } else {
        emitImplicit(VD, ICK_private);
      }
    } else if (CurrentDirectiveKind == OMPD_target) {
      // Normally all variables used in a target region are captured and
      // produce map clauses. We've disabled the captures for non-pointer
      // scalar variables so they will become firstprivate instead.
      if (OMPDeclareTargetDeclAttr::isDeclareTargetDeclaration(VD) &&
          VD->hasGlobalStorage() &&
          !CGF.getLangOpts().OpenMPDeclareTargetGlobalDefaultMap)
        emitImplicit(VD, ICK_livein);
      else if (Directive.getDirectiveKind() == OMPD_target &&
               (!VD->getType()->isScalarType() ||
                VD->getType()->isPointerType()))
        // Other not captured variables in target region should
        // be private.
        emitImplicit(VD, ICK_private);
      else
        emitImplicit(VD, ICK_firstprivate);
#if INTEL_CUSTOMIZATION
      if (OptRepFPMapInfos.find(VD) != OptRepFPMapInfos.end())
        emitRemark(OptRepFPMapInfos[VD]);
#endif  // INTEL_CUSTOMIZATION
    } else if (isImplicitTask(OMPD_task)) {
      // BE requests:
      // 1> Variables in the dispatch region are default shared in the
      //    implicit task.
      // 2> CapturedExpr with novariant/context clause is firstprivate in
      //    implicit task.
      // 3> Variable with is_dev_ptr is firstprivate in implicit task.
      // 4> Variable with firstprivate is firstprivate in implicit task.
      if (Directive.getDirectiveKind() == OMPD_dispatch &&
          !isa<OMPCapturedExprDecl>(VD) && !isDispatchExplicitVar(VD))
        emitImplicit(VD, ICK_shared);
      else
        emitImplicit(VD, ICK_firstprivate);
    } else if (CurrentDirectiveKind == OMPD_loop ||
               isAllowedClauseForDirective(CurrentDirectiveKind, OMPC_shared,
                                           CGF.getLangOpts().OpenMP)) {
      // Referenced but not defined in the region: shared
      emitImplicit(VD, ICK_shared);
    }
  }
  // Definitions of temps or uses of other values without representation in
  // the AST must be added.  We try to save these while generating code but
  // must use a ValueHandle since some Values are created but are not in the
  // final IR.  This is this is temporary until the back end can handle local
  // allocas correctly.
  for (auto &RV : ReferencedValues) {
    llvm::WeakTrackingVH &VH = RV.first;
    if (!VH.pointsToAliveValue())
      continue;
    llvm::Value *V = VH;
    llvm::Type *ElementType = RV.second;
    if (V->getName().find(".omp.iv") != StringRef::npos ||
        V->getName().find(".omp.ub") != StringRef::npos)
      continue;
    bool ValueFound = false;
    for (auto &DV : DefinedValues) {
      llvm::WeakTrackingVH &VH = DV.first;
      if (VH.pointsToAliveValue() && VH == V) {
        ValueFound = true;
        break;
      }
    }
    if (ValueFound) {
      // Defined in the region: private
      if (!alreadyHandled(V)) {
        ClauseEmissionHelper CEH(*this, OMPC_private, "QUAL.OMP.PRIVATE");
        ClauseStringBuilder &CSB = CEH.getBuilder();
        if (UseTypedClauses)
          CSB.setTyped();
        addArg(CSB.getString());
        addSingleElementTypedArg(V, ElementType, /*Handled=*/true);
      }
    } else if (CurrentDirectiveKind == OMPD_target) {
      if (!alreadyHandled(V)) {
        ClauseEmissionHelper CEH(*this, OMPC_firstprivate,
                                 "QUAL.OMP.FIRSTPRIVATE");
        ClauseStringBuilder &CSB = CEH.getBuilder();
        if (UseTypedClauses)
          CSB.setTyped();
        addArg(CSB.getString());
        addSingleElementTypedArg(V, ElementType, /*Handled=*/true);
      }
    } else if (CurrentDirectiveKind == OMPD_loop ||
               isAllowedClauseForDirective(CurrentDirectiveKind, OMPC_shared,
                                           CGF.getLangOpts().OpenMP)) {
      // Referenced but not defined in the region: shared
      if (!alreadyHandled(V))
        emitImplicit(V, ElementType, ICK_shared, /*Handled=*/true);
    }
  }
}

void OpenMPLateOutliner::addRefsToOuter() {
  if (CGF.CapturedStmtInfo) {
    for (const auto *VD : VarDefs) {
      if (isIgnoredImplicit(VD))
        continue;
      CGF.CapturedStmtInfo->recordVariableDefinition(VD);
    }
    for (const auto *VD : VarRefs) {
      if (isIgnoredImplicit(VD))
        continue;
      CGF.CapturedStmtInfo->recordVariableReference(VD);
    }
    for (const auto &It : ExplicitRefs) {
      CGF.CapturedStmtInfo->recordVariableReference(It.first);
      if (CurrentDirectiveKind == OMPD_dispatch) {
        for (const auto &CK : It.second)
          if (CK == OMPC_is_device_ptr || CK == OMPC_firstprivate) {
            CGF.CapturedStmtInfo->recordDispatchExplicitVar(It.first);
            break;
          }
      }
    }
    for (auto &DV : DefinedValues) {
      llvm::WeakTrackingVH &VH = DV.first;
      if (VH.pointsToAliveValue())
        CGF.CapturedStmtInfo->recordValueDefinition(DV.first, DV.second);
    }
    for (auto &RV : ReferencedValues) {
      llvm::WeakTrackingVH &VH = RV.first;
      if (VH.pointsToAliveValue())
        CGF.CapturedStmtInfo->recordValueReference(RV.first, RV.second);
    }
  }
}

const VarDecl *OpenMPLateOutliner::getExplicitVarDecl(const Expr *E) {
  if (const DeclRefExpr *DRE = getExplicitDeclRefOrNull(E))
    return cast<VarDecl>(DRE->getDecl());
  return nullptr;
}

const DeclRefExpr *OpenMPLateOutliner::getExplicitDeclRefOrNull(const Expr *E) {
  const Expr *ExplicitVarExpr = E;
  if (isa<ArraySubscriptExpr>(E->IgnoreParenImpCasts()) ||
      E->getType()->isSpecificPlaceholderType(BuiltinType::OMPArraySection)) {
    ExplicitVarExpr = OpenMPLateOutliner::getArraySectionBase(E);
  }
  if (auto *DRE = dyn_cast<DeclRefExpr>(ExplicitVarExpr))
    return DRE;
  return nullptr;
}

void OpenMPLateOutliner::emitOMPSharedClause(const OMPSharedClause *Cl) {
  for (auto *E : Cl->varlists()) {
    // Shared fields (or fields generated for lambda captures) are not
    // emitted since they are correctly handled through the shared this
    // pointer.
    if (const auto *ME = dyn_cast<MemberExpr>(E)) {
      if (isa<CXXThisExpr>(ME->getBase()))
        continue;
    } else if (const auto *DRE = dyn_cast<DeclRefExpr>(E)) {
      if (DRE->refersToEnclosingVariableOrCapture())
        continue;
    }
    const VarDecl *VD = getExplicitVarDecl(E);
    assert(VD && "expected VarDecl in shared clause");
    addExplicit(VD, OMPC_shared);

    ClauseEmissionHelper CEH(*this, OMPC_shared, "QUAL.OMP.SHARED");
    ClauseStringBuilder &CSB = CEH.getBuilder();

    bool IsRef = VD->getType()->isReferenceType();
    if (IsRef)
      CSB.setByRef();
    if (UseTypedClauses)
      CSB.setTyped();
    addArg(CSB.getString());
    addTypedArg(E, IsRef);
  }
}

void OpenMPLateOutliner::emitOMPPrivateClause(const OMPPrivateClause *Cl) {
  // Private clauses may generate routines used in target region so
  // setup the TargetRegion context if needed.
  bool IsDeviceTarget =
      CGF.getLangOpts().OpenMPIsDevice &&
      (CGF.CGM.inTargetRegion() ||
       isOpenMPTargetExecutionDirective(Directive.getDirectiveKind()));
  CodeGenModule::InTargetRegionRAII ITR(CGF.CGM, IsDeviceTarget);

  auto IPriv = Cl->private_copies().begin();
  for (auto *E : Cl->varlists()) {
    const VarDecl *VD = getExplicitVarDecl(E);
    assert(VD && "expected VarDecl in private clause");
    addExplicit(VD, OMPC_private);
    bool IsCapturedExpr = isa<OMPCapturedExprDecl>(VD);
    bool IsRef = !IsCapturedExpr && VD->getType()->isReferenceType();
    auto *Private = cast<VarDecl>(cast<DeclRefExpr>(*IPriv)->getDecl());
    const Expr *Init = Private->getInit();
    ClauseEmissionHelper CEH(*this, OMPC_private, "QUAL.OMP.PRIVATE");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    if (Init || Private->getType().isDestructedType())
      CSB.setNonPod();
    if (IsRef)
      CSB.setByRef();
    if (UseTypedClauses)
      CSB.setTyped();
    addArg(CSB.getString());
    addTypedArg(E, IsRef);
    if (Init || Private->getType().isDestructedType()) {
      addArg(emitOpenMPDefaultConstructor(*IPriv));
      addArg(emitOpenMPDestructor(Private->getType()));
    }
    ++IPriv;
  }
}
void OpenMPLateOutliner::emitOMPLastprivateClause(
    const OMPLastprivateClause *Cl) {
  auto IPriv = Cl->private_copies().begin();
  auto ISrcExpr = Cl->source_exprs().begin();
  auto IDestExpr = Cl->destination_exprs().begin();
  auto IAssignOp = Cl->assignment_ops().begin();
  for (auto *E : Cl->varlists()) {
    const VarDecl *VD = getExplicitVarDecl(E);
    assert(VD && "expected VarDecl in lastprivate clause");
    addExplicit(VD, OMPC_lastprivate);
    bool IsPODType = E->getType().isPODType(CGF.getContext());
    bool IsCapturedExpr = isa<OMPCapturedExprDecl>(VD);
    bool IsRef = !IsCapturedExpr && VD->getType()->isReferenceType();
    ClauseEmissionHelper CEH(*this, OMPC_lastprivate, "QUAL.OMP.LASTPRIVATE");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    if (!IsPODType)
      CSB.setNonPod();
    if (IsRef)
      CSB.setByRef();
    if (Cl->getKind() == OMPC_LASTPRIVATE_conditional)
      CSB.setConditional();
    if (UseTypedClauses)
      CSB.setTyped();
    addArg(CSB.getString());
    addTypedArg(E, IsRef);
    if (!IsPODType) {
      addArg(emitOpenMPDefaultConstructor(*IPriv));
      addArg(emitOpenMPCopyAssign(E->getType(), *ISrcExpr, *IDestExpr,
                                  *IAssignOp));
      addArg(emitOpenMPDestructor(E->getType()));
    }
    ++IPriv;
    ++ISrcExpr;
    ++IDestExpr;
    ++IAssignOp;
  }
}

void OpenMPLateOutliner::emitOMPLinearClause(const OMPLinearClause *Cl) {
  for (auto *E : Cl->varlists()) {
    const VarDecl *VD = getExplicitVarDecl(E);
    assert(VD && "expected VarDecl in linear clause");
    addExplicit(VD, OMPC_linear);
    bool IsCapturedExpr = isa<OMPCapturedExprDecl>(VD);
    bool IsRef = !IsCapturedExpr && VD->getType()->isReferenceType();
    bool IsPtr = E->getType()->isPointerType();
    ClauseEmissionHelper CEH(*this, OMPC_linear, "QUAL.OMP.LINEAR");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    if (IsRef)
      CSB.setByRef();
    if (UseTypedClauses)
      CSB.setTyped();
    if (UseTypedClauses && IsPtr)
      CSB.setPtrToPtr();
    addArg(CSB.getString());
    if (UseTypedClauses && IsPtr) {
      QualType PointeeT = E->getType()->getPointeeType();
      llvm::Type *ElemTy = PointeeT->isFunctionType()
                               ? CGF.Int8Ty
                               : CGF.ConvertTypeForMem(PointeeT);
      addArg(E, IsRef, UseTypedClauses, /*NeedsTypedElements=*/true, ElemTy);
     } else
      addTypedArg(E, IsRef);
    addArg(Cl->getStep() ? CGF.EmitScalarExpr(Cl->getStep())
                         : CGF.Builder.getInt32(1));
  }
}

template <typename RedClause>
void OpenMPLateOutliner::emitOMPReductionClauseCommon(const RedClause *Cl,
                                                      StringRef QualName,
                                                      ImplicitClauseKind ICK) {
  // Reduction clauses may generate routines used in target region so
  // setup the TargetRegion context if needed.
  bool IsDeviceTarget =
      CGF.getLangOpts().OpenMPIsDevice &&
      (CGF.CGM.inTargetRegion() ||
       isOpenMPTargetExecutionDirective(Directive.getDirectiveKind()));
  CodeGenModule::InTargetRegionRAII ITR(CGF.CGM, IsDeviceTarget);

  auto I = Cl->reduction_ops().begin();
  auto IPriv = Cl->privates().begin();
  auto IRHS = Cl->rhs_exprs().begin();
  auto ILHS = Cl->lhs_exprs().begin();
  for (auto *E : Cl->varlists()) {
    const VarDecl *VD = getExplicitVarDecl(E);
    auto *Private = cast<VarDecl>(cast<DeclRefExpr>(*IPriv)->getDecl());
    assert(VD && "expected VarDecl in reduction clause");
    addExplicit(VD, Cl->getClauseKind());
    bool IsCapturedExpr = isa<OMPCapturedExprDecl>(VD);
    bool IsRef = !IsCapturedExpr && VD->getType()->isReferenceType();
    llvm::Value *InitFn = nullptr, *CombinerFn  = nullptr;
    bool IsUDR = false;
    if (const OMPDeclareReductionDecl *DRD = CGOpenMPRuntime::getRedInit(*I)) {
      IsUDR = true;
      // If device compile only generate routines used in target regions.
      if (!CGF.getLangOpts().OpenMPIsDevice || IsDeviceTarget) {
        std::pair<llvm::Function *, llvm::Function *> InitCombiner;
        InitCombiner = CGF.CGM.getOpenMPRuntime().getUserDefinedReduction(DRD);
        CombinerFn = InitCombiner.first;
        InitFn = InitCombiner.second;
      }
    } else if (!isa<BinaryOperator>((*I)->IgnoreImpCasts())) {
      IsUDR = true;
      // If device compile only generate routines used in target regions.
      if (!CGF.getLangOpts().OpenMPIsDevice || IsDeviceTarget) {
        const auto *LVD = cast<VarDecl>(cast<DeclRefExpr>(*ILHS)->getDecl());
        const auto *RVD = cast<VarDecl>(cast<DeclRefExpr>(*IRHS)->getDecl());
        CombinerFn =
            CGOpenMPRuntime::emitCombiner(CGF.CGM, VD->getType(), *I, RVD, LVD);
      }
    }
    OverloadedOperatorKind OOK =
        CombinerFn || IsUDR
            ? OO_None
            : Cl->getNameInfo().getName().getCXXOverloadedOperator();
    ClauseEmissionHelper CEH(*this, Cl->getClauseKind(), "QUAL.OMP.");
    CEH.setImplicitClause(ICK);
    ClauseStringBuilder &CSB = CEH.getBuilder();
    CSB.add(QualName);
    CSB.add(".");
    switch (OOK) {
    case OO_Plus:
      CSB.add("ADD");
      break;
    case OO_Minus:
      CSB.add("SUB");
      break;
    case OO_Star:
      CSB.add("MUL");
      break;
    case OO_Amp:
      CSB.add("BAND");
      break;
    case OO_Pipe:
      CSB.add("BOR");
      break;
    case OO_Caret:
      CSB.add("BXOR");
      break;
    case OO_AmpAmp:
      CSB.add("AND");
      break;
    case OO_PipePipe:
      CSB.add("OR");
      break;
    case OO_New:
    case OO_Delete:
    case OO_Array_New:
    case OO_Array_Delete:
    case OO_Slash:
    case OO_Percent:
    case OO_Tilde:
    case OO_Exclaim:
    case OO_Equal:
    case OO_Less:
    case OO_Greater:
    case OO_LessEqual:
    case OO_GreaterEqual:
    case OO_Spaceship:
    case OO_PlusEqual:
    case OO_MinusEqual:
    case OO_StarEqual:
    case OO_SlashEqual:
    case OO_PercentEqual:
    case OO_CaretEqual:
    case OO_AmpEqual:
    case OO_PipeEqual:
    case OO_LessLess:
    case OO_GreaterGreater:
    case OO_LessLessEqual:
    case OO_GreaterGreaterEqual:
    case OO_EqualEqual:
    case OO_ExclaimEqual:
    case OO_PlusPlus:
    case OO_MinusMinus:
    case OO_Comma:
    case OO_ArrowStar:
    case OO_Arrow:
    case OO_Call:
    case OO_Subscript:
    case OO_Conditional:
    case OO_Coawait:
    case NUM_OVERLOADED_OPERATORS:
      llvm_unreachable("Unexpected reduction identifier");
    case OO_None:
      if (CombinerFn || IsUDR) {
        CSB.add("UDR");
      } else if (auto II = Cl->getNameInfo().getName().getAsIdentifierInfo()) {
        if (II->isStr("max"))
          CSB.add("MAX");
        else if (II->isStr("min"))
          CSB.add("MIN");
        QualType ElemType = E->getType();
        if (ElemType->isArrayType())
          ElemType = CGF.CGM.getContext()
                         .getBaseElementType(ElemType)
                         .getNonReferenceType();
        if (ElemType->isVectorType())
          ElemType = ElemType->getAs<VectorType>()->getElementType();
        if (ElemType->isUnsignedIntegerType())
          CSB.setUnsigned();
      }
      break;
    }
    if (const auto *C = dyn_cast<OMPReductionClause>(Cl))
      switch (C->getModifier()) {
      case OMPC_REDUCTION_task:
        CSB.setTask();
        break;
      case OMPC_REDUCTION_inscan:
        CSB.setInScan();
        break;
      case OMPC_REDUCTION_default:
      case OMPC_REDUCTION_unknown:
        break;
      }
    if (CGF.CGM.getContext()
            .getBaseElementType(E->getType())
            .getNonReferenceType()
            ->getPointeeOrArrayElementType()
            ->isAnyComplexType())
      CSB.setCmplx();
    if (IsRef)
      CSB.setByRef();
    llvm::Type *ElemTy = nullptr;
    if (isa<ArraySubscriptExpr>(E->IgnoreParenImpCasts()) ||
        E->getType()->isSpecificPlaceholderType(BuiltinType::OMPArraySection)) {
      CSB.setArrSect();
      QualType BaseTy = getArraySectionBase(E)->getType();
      if (isa<ComplexType>(BaseTy->getPointeeOrArrayElementType()))
        CSB.setCmplx();
      if (UseTypedClauses && BaseTy->isPointerType()) {
        CSB.setPtrToPtr();
        QualType PointeeTy = BaseTy->getPointeeType();
        if (PointeeTy->isArrayType())
          PointeeTy = QualType(PointeeTy->getPointeeOrArrayElementType(), 0);
        ElemTy = CGF.ConvertTypeForMem(PointeeTy);
      }
    }
    if (UseTypedClauses)
      CSB.setTyped();
    addArg(CSB.getString());
    if (ElemTy)
      addArg(E, IsRef, UseTypedClauses, /*NeedsTypedElements=*/true, ElemTy);
    else
      addTypedArg(E, IsRef);
    if (CombinerFn) {
      llvm::Value *Cons = llvm::ConstantPointerNull::get(CGF.VoidPtrTy);
      llvm::Value *Des = llvm::ConstantPointerNull::get(CGF.VoidPtrTy);
      llvm::Value *Init =
          InitFn ? InitFn : llvm::ConstantPointerNull::get(CGF.VoidPtrTy);
      if (Private->getInit() || Private->getType().isDestructedType()) {
        if (!InitFn)
          Cons = emitOpenMPDefaultConstructor(*IPriv, /*IsUDR=*/true);
        Des = emitOpenMPDestructor(Private->getType(), /*IsUDR=*/true);
      }
      for (auto *FV : {Cons, Des, CombinerFn, Init})
        addArg(FV);
    }
    if (const auto *C = dyn_cast<OMPReductionClause>(Cl))
      if (C->getModifier() == OMPC_REDUCTION_inscan) {
        CGF.addInscanVar(VD);
        addArg(llvm::ConstantInt::get(CGF.SizeTy, CGF.getInscanVarIndex(VD)));
      }
    ++I;
    ++IPriv;
    ++ILHS;
    ++IRHS;
  }
}

void OpenMPLateOutliner::emitOMPReductionClause(const OMPReductionClause *Cl) {
  if (CodeGenFunction::requiresImplicitTaskgroup(Directive,
                                                 /*TopLevel=*/false)) {
    // Implicit taskgroups cause an INREDUCTION on the taskloop piece of the
    // full directive.  Add both here and sort out which one to use in
    // getApplicableDirectives.
    emitOMPReductionClauseCommon(Cl, "REDUCTION", ICK_reduction);
    emitOMPReductionClauseCommon(Cl, "INREDUCTION", ICK_inreduction);
    return;
  }
  emitOMPReductionClauseCommon(Cl, "REDUCTION");
}

void OpenMPLateOutliner::emitOMPTaskReductionClause(
    const OMPTaskReductionClause *Cl) {
  emitOMPReductionClauseCommon(Cl, "REDUCTION");
}

void OpenMPLateOutliner::emitOMPInReductionClause(
    const OMPInReductionClause *Cl) {
  emitOMPReductionClauseCommon(Cl, "INREDUCTION");
}

void OpenMPLateOutliner::emitOMPOrderedClause(const OMPOrderedClause *C) {
  ClauseEmissionHelper CEH(*this, OMPC_ordered);
  addArg("QUAL.OMP.ORDERED");
  if (auto *E = C->getNumForLoops())
    addArg(CGF.EmitScalarExpr(E));
  else
    addArg(CGF.Builder.getInt32(0));
  for (auto *LNI : C->getLoopNumIterations())
    addArg(CGF.EmitScalarExpr(LNI));
}

void OpenMPLateOutliner::emitOMPScheduleClause(const OMPScheduleClause *C) {
  int DefaultChunkSize = 0;
  ClauseEmissionHelper CEH(*this, OMPC_schedule, "QUAL.OMP.SCHEDULE.");
  ClauseStringBuilder &CSB = CEH.getBuilder();
  switch (C->getScheduleKind()) {
  case OMPC_SCHEDULE_static:
    CSB.add("STATIC");
    break;
  case OMPC_SCHEDULE_dynamic:
    DefaultChunkSize = 1;
    CSB.add("DYNAMIC");
    break;
  case OMPC_SCHEDULE_guided:
    DefaultChunkSize = 1;
    CSB.add("GUIDED");
    break;
  case OMPC_SCHEDULE_auto:
    CSB.add("AUTO");
    break;
  case OMPC_SCHEDULE_runtime:
    CSB.add("RUNTIME");
    break;
  case OMPC_SCHEDULE_unknown:
    llvm_unreachable("Unknown schedule clause");
  }

  for (int Count = 0; Count < 2; ++Count) {
    auto Mod = Count == 0 ? C->getFirstScheduleModifier()
                          : C->getSecondScheduleModifier();
    switch (Mod) {
    case OMPC_SCHEDULE_MODIFIER_monotonic:
      CSB.setMonotonic();
      break;
    case OMPC_SCHEDULE_MODIFIER_nonmonotonic:
      CSB.setNonMonotonic();
      break;
    case OMPC_SCHEDULE_MODIFIER_simd:
      CSB.setSimd();
      break;
    case OMPC_SCHEDULE_MODIFIER_last:
    case OMPC_SCHEDULE_MODIFIER_unknown:
      break;
    }
  }
  addArg(CSB.getString());
  if (auto *E = C->getChunkSize())
    addArg(CGF.EmitScalarExpr(E));
  else
    addArg(CGF.Builder.getInt32(DefaultChunkSize));
}

void OpenMPLateOutliner::emitOMPFirstprivateClause(
    const OMPFirstprivateClause *Cl) {
  // Firstprivate clauses may generate routines used in target region so
  // setup the TargetRegion context if needed.
  bool IsDeviceTarget =
      CGF.getLangOpts().OpenMPIsDevice &&
      (CGF.CGM.inTargetRegion() ||
       isOpenMPTargetExecutionDirective(Directive.getDirectiveKind()));
  CodeGenModule::InTargetRegionRAII ITR(CGF.CGM, IsDeviceTarget);

  auto *IPriv = Cl->private_copies().begin();
  for (auto *E : Cl->varlists()) {
    const VarDecl *VD = getExplicitVarDecl(E);

    // Handle implicit firstprivates added to target directives.
    if (Cl->isImplicit() &&
        isOpenMPTargetExecutionDirective(Directive.getDirectiveKind())) {
      if (CurrentDirectiveKind == OMPD_target) {
        // The toplevel target can be added to the implicit list and handled
        // during normal implicit clause handling.
        ImplicitMap.insert(std::make_pair(VD, ICK_firstprivate));
        continue;
      }
      // For non-target parts of the directive an implicit firstprivate must be
      // ignored unless it is from a default(firstprivate).
      const auto *DC = Directive.getSingleClause<OMPDefaultClause>();
      if (!DC || DC->getDefaultKind() != OMP_DEFAULT_firstprivate)
        continue;
    }

    ClauseEmissionHelper CEH(*this, OMPC_firstprivate, "QUAL.OMP.FIRSTPRIVATE");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    assert(VD && "expected VarDecl in firstprivate clause");
    addExplicit(VD, OMPC_firstprivate);
    bool IsPODType = E->getType().isPODType(CGF.getContext());
    bool IsCapturedExpr = isa<OMPCapturedExprDecl>(VD);
    bool IsRef = !IsCapturedExpr && VD->getType()->isReferenceType();
    if (!IsPODType)
      CSB.setNonPod();
    if (IsRef)
      CSB.setByRef();
    if (UseTypedClauses)
      CSB.setTyped();
    addArg(CSB.getString());
    addTypedArg(E, IsRef);
    if (!IsPODType) {
      addArg(emitOpenMPCopyConstructor(*IPriv));
      addArg(emitOpenMPDestructor(E->getType()));
    }
    ++IPriv;
  }
}

void OpenMPLateOutliner::emitOMPCopyinClause(const OMPCopyinClause *Cl) {
  for (auto *E : Cl->varlists()) {
    ClauseEmissionHelper CEH(*this, OMPC_copyin, "QUAL.OMP.COPYIN");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    if (!E->getType().isPODType(CGF.getContext()))
      CGF.CGM.ErrorUnsupported(E, "non-POD copyin variable");
    const VarDecl *VD = getExplicitVarDecl(E);
    assert(VD && "expected VarDecl in copyin clause");
    addExplicit(VD, OMPC_copyin);
    if (UseTypedClauses)
      CSB.setTyped();
    addArg(CSB.getString());
    addTypedArg(E);
  }
}

void OpenMPLateOutliner::emitOMPIfClause(const OMPIfClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_if);
  addArg("QUAL.OMP.IF");
  addArg(CGF.EvaluateExprAsBool(Cl->getCondition()));
}

void OpenMPLateOutliner::emitOMPNumThreadsClause(
    const OMPNumThreadsClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_num_threads);
  addArg("QUAL.OMP.NUM_THREADS");
  addArg(CGF.EmitScalarExpr(Cl->getNumThreads()));
}

void OpenMPLateOutliner::emitOMPFilterClause(const OMPFilterClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_filter);
  addArg("QUAL.OMP.FILTER");
  addArg(CGF.EmitScalarExpr(Cl->getThreadID()));
}

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
void OpenMPLateOutliner::emitOMPDataflowClause(const OMPDataflowClause *Cl) {
  if (auto *E = Cl->getStaticChunkSize()) {
    ClauseEmissionHelper CEH(*this, OMPC_dataflow);
    addArg("QUAL.OMP.SA.SCHEDULE.STATIC");
    addArg(CGF.EmitScalarExpr(E));
  }
  if (auto *E = Cl->getNumWorkersNum()) {
    ClauseEmissionHelper CEH(*this, OMPC_dataflow);
    addArg("QUAL.OMP.SA.NUM_WORKERS");
    addArg(CGF.EmitScalarExpr(E));
  }
  if (auto *E = Cl->getPipelineDepth()) {
    ClauseEmissionHelper CEH(*this, OMPC_dataflow);
    addArg("QUAL.OMP.SA.PIPELINE");
    addArg(CGF.EmitScalarExpr(E));
  }
}
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION

void OpenMPLateOutliner::emitOMPDefaultClause(const OMPDefaultClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_default);
  switch (Cl->getDefaultKind()) {
  case OMP_DEFAULT_none:
    addArg("QUAL.OMP.DEFAULT.NONE");
    break;
  case OMP_DEFAULT_shared:
    addArg("QUAL.OMP.DEFAULT.SHARED");
    break;
  case OMP_DEFAULT_firstprivate:
    addArg("QUAL.OMP.DEFAULT.FIRSTPRIVATE");
    break;
  case OMP_DEFAULT_private:
    addArg("QUAL.OMP.DEFAULT.PRIVATE");
    break;
  case OMP_DEFAULT_unknown:
    llvm_unreachable("Unknown default clause");
  }
}

void OpenMPLateOutliner::emitOMPProcBindClause(const OMPProcBindClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_proc_bind);
  switch (Cl->getProcBindKind()) {
  case OMP_PROC_BIND_master:
    addArg("QUAL.OMP.PROC_BIND.MASTER");
    break;
  case OMP_PROC_BIND_close:
    addArg("QUAL.OMP.PROC_BIND.CLOSE");
    break;
  case OMP_PROC_BIND_spread:
    addArg("QUAL.OMP.PROC_BIND.SPREAD");
    break;
  case OMP_PROC_BIND_primary: // Not yet implemented
  case OMP_PROC_BIND_default:
    break;
  case OMP_PROC_BIND_unknown:
    llvm_unreachable("Unknown proc_bind clause");
  }
}

void OpenMPLateOutliner::emitOMPSafelenClause(const OMPSafelenClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_safelen);
  addArg("QUAL.OMP.SAFELEN");
  addArg(CGF.EmitScalarExpr(Cl->getSafelen()));
}

void OpenMPLateOutliner::emitOMPSimdlenClause(const OMPSimdlenClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_simdlen);
  addArg("QUAL.OMP.SIMDLEN");
  addArg(CGF.EmitScalarExpr(Cl->getSimdlen()));
}

void OpenMPLateOutliner::emitOMPCollapseClause(const OMPCollapseClause *Cl) {
#if INTEL_CUSTOMIZATION
  // Don't emit implicit collapse clause when tile clause present.
  if (Directive.hasClausesOfKind<OMPTileClause>())
    return;
#endif // INTEL_CUSTOMIZATION
  ClauseEmissionHelper CEH(*this, OMPC_collapse);
  addArg("QUAL.OMP.COLLAPSE");
  addArg(CGF.EmitScalarExpr(Cl->getNumForLoops()));
}

void OpenMPLateOutliner::emitOMPAlignedClause(const OMPAlignedClause *Cl) {
  for (auto *E : Cl->varlists()) {
    ClauseEmissionHelper CEH(*this, OMPC_aligned, "QUAL.OMP.ALIGNED");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    E = E->IgnoreParenImpCasts();
    if (E->getType()->isPointerType())
      CSB.setPtrToPtr();
    addArg(CSB.getString());
    addArg(E);
    addArg(Cl->getAlignment() ? CGF.EmitScalarExpr(Cl->getAlignment())
                              : CGF.Builder.getInt32(0));
  }
}

void OpenMPLateOutliner::emitOMPGrainsizeClause(const OMPGrainsizeClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_grainsize);
  addArg("QUAL.OMP.GRAINSIZE");
  addArg(CGF.EmitScalarExpr(Cl->getGrainsize()));
}

void OpenMPLateOutliner::emitOMPNumTasksClause(const OMPNumTasksClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_num_tasks);
  addArg("QUAL.OMP.NUM_TASKS");
  addArg(CGF.EmitScalarExpr(Cl->getNumTasks()));
}

void OpenMPLateOutliner::emitOMPPriorityClause(const OMPPriorityClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_priority);
  addArg("QUAL.OMP.PRIORITY");
  addArg(CGF.EmitScalarExpr(Cl->getPriority()));
}

void OpenMPLateOutliner::emitOMPFinalClause(const OMPFinalClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_final);
  addArg("QUAL.OMP.FINAL");
  addArg(CGF.EvaluateExprAsBool(Cl->getCondition()));
}

void OpenMPLateOutliner::emitOMPNogroupClause(const OMPNogroupClause *) {
  ClauseEmissionHelper CEH(*this, OMPC_nogroup);
  addArg("QUAL.OMP.NOGROUP");
}

void OpenMPLateOutliner::emitOMPMergeableClause(const OMPMergeableClause *) {
  ClauseEmissionHelper CEH(*this, OMPC_mergeable);
  addArg("QUAL.OMP.MERGEABLE");
}

void OpenMPLateOutliner::emitOMPUntiedClause(const OMPUntiedClause *) {
  ClauseEmissionHelper CEH(*this, OMPC_untied);
  addArg("QUAL.OMP.UNTIED");
}

void OpenMPLateOutliner::emitOMPDependClause(const OMPDependClause *Cl) {
  // This function is needed until old IR for depend clause is no longer
  // necessary and flag has been removed.
  if (CGF.getLangOpts().OpenMPNewDependIR)
    return;
  auto DepKind = Cl->getDependencyKind();
  if (DepKind == OMPC_DEPEND_source || DepKind == OMPC_DEPEND_sink) {
    ClauseEmissionHelper CEH(*this, OMPC_depend);
    if (DepKind == OMPC_DEPEND_source)
      addArg("QUAL.OMP.DEPEND.SOURCE");
    else
      addArg("QUAL.OMP.DEPEND.SINK");
    for (unsigned I = 0, E = Cl->getNumLoops(); I < E; ++I)
      addArg(CGF.EmitScalarExpr(Cl->getLoopData(I)));
    return;
  }

  for (auto *E : Cl->varlists()) {
    ClauseEmissionHelper CEH(*this, OMPC_depend);
    ClauseStringBuilder &CSB = CEH.getBuilder();
    switch (DepKind) {
    case OMPC_DEPEND_in:
      CSB.add("QUAL.OMP.DEPEND.IN");
      break;
    case OMPC_DEPEND_out:
      CSB.add("QUAL.OMP.DEPEND.OUT");
      break;
    case OMPC_DEPEND_inout:
      CSB.add("QUAL.OMP.DEPEND.INOUT");
      break;
    default:
      llvm_unreachable("Unknown depend clause");
    }
    if (isa<ArraySubscriptExpr>(E->IgnoreParenImpCasts()) ||
        E->getType()->isSpecificPlaceholderType(BuiltinType::OMPArraySection))
      CSB.setArrSect();
    addArg(CSB.getString());
    addArg(E);
  }
}

void OpenMPLateOutliner::emitOMPDeviceClause(const OMPDeviceClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_device);
  addArg("QUAL.OMP.DEVICE");
  addArg(CGF.EmitScalarExpr(Cl->getDevice()));
}

void OpenMPLateOutliner::emitOMPIsDevicePtrClause(
    const OMPIsDevicePtrClause *Cl) {
  if (CurrentDirectiveKind == OMPD_target)
    return;
  ClauseEmissionHelper CEH(*this, OMPC_is_device_ptr);
  ClauseStringBuilder &CSB = CEH.getBuilder();
  CSB.add("QUAL.OMP.IS_DEVICE_PTR");
  CSB.setPtrToPtr();
  addArg(CSB.getString());
  for (const auto *E : Cl->varlists()) {
    const VarDecl *VD = getExplicitVarDecl(E);
    assert(VD && "expected VarDecl in is_device_ptr clause");
    addExplicit(VD, OMPC_is_device_ptr);
    addArg(E);
  }
}

void OpenMPLateOutliner::emitOMPSubdeviceClause(const OMPSubdeviceClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_subdevice);
  ClauseStringBuilder &CSB = CEH.getBuilder();
  CSB.add("QUAL.OMP.SUBDEVICE");
  addArg(CSB.getString());
  addArg(CGF.EmitScalarExpr(Cl->getLevel()));
  addArg(CGF.EmitScalarExpr(Cl->getStart()));
  addArg(CGF.EmitScalarExpr(Cl->getLength()));
  addArg(CGF.EmitScalarExpr(Cl->getStride()));
}

void OpenMPLateOutliner::emitOMPOmpxPlacesClause(
    const OMPOmpxPlacesClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_ompx_places);
  ClauseStringBuilder &CSB = CEH.getBuilder();
  CSB.add("QUAL.OMP.SUBDEVICE");
  addArg(CSB.getString());

  // Determine domain-modifier value and default values for any missing
  // arguments.
  switch (Cl->getModifier()) {
  case OMPC_OMPX_PLACES_numa_domain:
  // Default domain-modifier value is same as explicit numa_domain.
  case OMPC_OMPX_PLACES_unknown:
    addArg(CGF.Builder.getInt32(0));
    break;
  case OMPC_OMPX_PLACES_subnuma_domain:
    addArg(CGF.Builder.getInt32(1));
    break;
  }
  assert(Cl->getStart());
  addArg(CGF.EmitScalarExpr(Cl->getStart()));

  // length and stride default to 1 if not specified in ompx_places.
  if (Cl->getLength())
    addArg(CGF.EmitScalarExpr(Cl->getLength()));
  else
    addArg(CGF.Builder.getInt32(1));
  if (Cl->getStride())
    addArg(CGF.EmitScalarExpr(Cl->getStride()));
  else
    addArg(CGF.Builder.getInt32(1));
}

void OpenMPLateOutliner::emitOMPDefaultmapClause(
    const OMPDefaultmapClause *Cl) {

  ClauseEmissionHelper CEH(*this, OMPC_defaultmap);
  ClauseStringBuilder &CSB = CEH.getBuilder();
  CSB.add("QUAL.OMP.DEFAULTMAP.");
  switch (Cl->getDefaultmapModifier()) {
  case OMPC_DEFAULTMAP_MODIFIER_alloc:
    CSB.add("ALLOC");
    break;
  case OMPC_DEFAULTMAP_MODIFIER_to:
    CSB.add("TO");
    break;
  case OMPC_DEFAULTMAP_MODIFIER_from:
    CSB.add("FROM");
    break;
  case OMPC_DEFAULTMAP_MODIFIER_tofrom:
    CSB.add("TOFROM");
    break;
  case OMPC_DEFAULTMAP_MODIFIER_default:
    CSB.add("DEFAULT");
    break;
  case OMPC_DEFAULTMAP_MODIFIER_none:
    CSB.add("NONE");
    break;
  case OMPC_DEFAULTMAP_MODIFIER_firstprivate:
    CSB.add("FIRSTPRIVATE");
    break;
  case OMPC_DEFAULTMAP_MODIFIER_present:
    CSB.add("PRESENT");
    break;
  default:
     llvm_unreachable("Unexpected defaultmap modifier");
  }
  switch (Cl->getDefaultmapKind()) {
  case OMPC_DEFAULTMAP_scalar:
    CSB.add(":SCALAR");
    break;
  case OMPC_DEFAULTMAP_aggregate:
    CSB.add(":AGGREGATE");
    break;
  case OMPC_DEFAULTMAP_pointer:
    CSB.add(":POINTER");
    break;
  case OMPC_DEFAULTMAP_unknown:
    break;
  }
  addArg(CSB.getString());
}

void OpenMPLateOutliner::emitOMPNowaitClause(const OMPNowaitClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_nowait);
  addArg("QUAL.OMP.NOWAIT");
}

void OpenMPLateOutliner::emitOMPUseDevicePtrClause(
    const OMPUseDevicePtrClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_use_device_ptr);
  addArg("QUAL.OMP.USE_DEVICE_PTR:PTR_TO_PTR");
  for (auto *E : Cl->varlists())
    addArg(E);
}

void OpenMPLateOutliner::emitOMPNumTeamsClause(const OMPNumTeamsClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_num_teams, "QUAL.OMP.NUM_TEAMS");
  ClauseStringBuilder &CSB = CEH.getBuilder();
  Address A = CGF.getMappedClause(Cl);
  if (A.isValid()) {
    addValueRef(A.getPointer(), A.getElementType());
    if (UseTypedClauses)
      CSB.setTyped();
    addArg(CSB.getString());
    addNoElementTypedArg(A.getPointer(), A.getElementType(), /*Handled=*/true);
  } else {
    addArg(CSB.getString());
    addArg(CGF.EmitScalarExpr(Cl->getNumTeams()));
  }
}

void OpenMPLateOutliner::emitOMPThreadLimitClause(
    const OMPThreadLimitClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_thread_limit, "QUAL.OMP.THREAD_LIMIT");
  ClauseStringBuilder &CSB = CEH.getBuilder();
  Address A = CGF.getMappedClause(Cl);
  if (A.isValid()) {
    addValueRef(A.getPointer(), A.getElementType());
    if (UseTypedClauses)
      CSB.setTyped();
    addArg(CSB.getString());
    addNoElementTypedArg(A.getPointer(), A.getElementType(), /*Handled=*/true);
  } else {
    addArg(CSB.getString());
    addArg(CGF.EmitScalarExpr(Cl->getThreadLimit()));
  }
}

void OpenMPLateOutliner::emitOMPDistScheduleClause(
    const OMPDistScheduleClause *Cl) {
  if (Cl->getDistScheduleKind() != OMPC_DIST_SCHEDULE_static)
    llvm_unreachable("Unsupported dist_schedule clause");

  int DefaultChunkSize = 0;
  ClauseEmissionHelper CEH(*this, OMPC_dist_schedule);
  addArg("QUAL.OMP.DIST_SCHEDULE.STATIC");
  if (auto *E = Cl->getChunkSize())
    addArg(CGF.EmitScalarExpr(E));
  else
    addArg(CGF.Builder.getInt32(DefaultChunkSize));
}

void OpenMPLateOutliner::emitOMPFlushClause(const OMPFlushClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_flush);
  addArg("QUAL.OMP.FLUSH");
  for (auto *E : Cl->varlists())
    addArg(E);
}

void OpenMPLateOutliner::emitOMPCopyprivateClause(
    const OMPCopyprivateClause *Cl) {
  auto ISrcExpr = Cl->source_exprs().begin();
  auto IDestExpr = Cl->destination_exprs().begin();
  auto IAssignOp = Cl->assignment_ops().begin();
  for (auto *E : Cl->varlists()) {
    ClauseEmissionHelper CEH(*this, OMPC_copyprivate, "QUAL.OMP.COPYPRIVATE");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    bool IsPODType = E->getType().isPODType(CGF.getContext());
    const VarDecl *VD = getExplicitVarDecl(E);
    assert(VD && "expected VarDecl in copyprivate clause");
    addExplicit(VD, OMPC_copyprivate);
    if (!IsPODType)
      CSB.setNonPod();
    if (UseTypedClauses)
      CSB.setTyped();
    addArg(CSB.getString());
    addTypedArg(E);
    if (!IsPODType) {
      addArg(emitOpenMPCopyAssign(E->getType(), *ISrcExpr, *IDestExpr,
                                  *IAssignOp));
    }
    ++ISrcExpr;
    ++IDestExpr;
    ++IAssignOp;
  }
}

void OpenMPLateOutliner::emitOMPHintClause(const OMPHintClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_hint);
  addArg("QUAL.OMP.HINT");
  addArg(CGF.EmitScalarExpr(Cl->getHint()));
}

void OpenMPLateOutliner::buildMapQualifier(
   ClauseStringBuilder &CSB,
   OpenMPMapClauseKind MapType,
   const SmallVector<OpenMPMapModifierKind, 1>  Modifiers,
   const VarDecl *MapVar) {
  CSB.add("QUAL.OMP.MAP.");
  switch (MapType) {
  case OMPC_MAP_alloc:
    CSB.add("ALLOC");
    break;
  case OMPC_MAP_to:
    CSB.add("TO");
    break;
  case OMPC_MAP_from:
    CSB.add("FROM");
    break;
  case OMPC_MAP_tofrom:
  case OMPC_MAP_unknown:
    CSB.add("TOFROM");
    break;
  case OMPC_MAP_delete:
    CSB.add("DELETE");
    break;
  case OMPC_MAP_release:
    CSB.add("RELEASE");
    break;
  }
  for (auto MD : Modifiers) {
    switch (MD) {
      case OMPC_MAP_MODIFIER_always:
        CSB.setAlways();
        break;
      case OMPC_MAP_MODIFIER_close:
        CSB.setClose();
        break;
      case OMPC_MAP_MODIFIER_present:
        CSB.setPresent();
        break;
      case OMPC_MAP_MODIFIER_unknown:
        break;
      default:
        llvm_unreachable("Unexpected map modifier");
    }
  }
  // OpenMP5.1 pg 254 lines 8-10
  //   The in_reduction clause applies to the single leaf construct on which
  //   it is permitted. If that construct is a target construct, the effect is
  //   as if the same list item also appears in a map clause with a map-type
  //   of tofrom and a map-type-modifier of always.
  if (isImplicitTask(OMPD_target) && MapVar)
    for (const auto *C : Directive.getClausesOfKind<OMPInReductionClause>()) {
      const auto CIt = std::find_if(
          C->varlists().begin(), C->varlists().end(),
          [&MapVar](const Expr *E) {
            return getExplicitVarDecl(E) == MapVar;
          });
      if (CIt != C->varlists().end()) {
        assert(MapType == OMPC_MAP_tofrom &&
               "wrong map type for target in_reduction variable");
        CSB.setAlways();
        break;
      }
    }
}

#if INTEL_CUSTOMIZATION
namespace {
class ExprVarRefFinder final : public ConstStmtVisitor<ExprVarRefFinder> {
  CodeGenFunction &CGF;
  llvm::MapVector<const VarDecl *, std::string> *FPInfos;
  llvm::SmallVector<const Expr *> MapVarExprs;
public:
  llvm::SmallVector<const Expr *> &getMapVarExprs() { return MapVarExprs; }
  void VisitDeclRefExpr(const DeclRefExpr *E) {
    const auto *VD = dyn_cast<VarDecl>(E->getDecl());
    MapVarExprs.push_back(E);
    if (VD && VD->getType()->isScalarType() &&
        !VD->getType()->isPointerType()) {
      PresumedLoc PLoc = CGF.getContext().getSourceManager().getPresumedLoc(
          E->getExprLoc());
      const auto *VD = dyn_cast<VarDecl>(dyn_cast<DeclRefExpr>(E)->getDecl());
      unsigned Line = PLoc.getLine();
      unsigned Column = PLoc.getColumn();
      std::string Name = VD->getNameAsString();
      std::string ReasonStr;
      ReasonStr = "  \"" + Name + "\" has an implicit clause: \"firstprivate(" +
                  Name + ")\" because \"" + Name + "\" is a scalar variable ";
      ReasonStr += "referenced within the construct at line:[" +
                   std::to_string(Line) + ":" + std::to_string(Column) + "]";
      FPInfos->insert(std::make_pair(VD, ReasonStr));
    }
  }
  void VisitMemberExpr(const MemberExpr *ME) {
    MapVarExprs.push_back(ME);
    Visit(ME->getBase());
  }
  void VisitCXXThisExpr(const CXXThisExpr *CTE) {
    MapVarExprs.push_back(CTE);
  }
  void VisitStmt(const Stmt *S) {
    for (const Stmt *Child : S->children())
      if (Child)
        Visit(Child);
  }
  ExprVarRefFinder(CodeGenFunction &CGF,
                   llvm::MapVector<const VarDecl *, std::string> *FPInfos)
      : CGF(CGF), FPInfos(FPInfos) {}
};
}
static std::string getReasonStr(const ValueDecl *Var, CodeGenFunction &CGF,
                                ImplicitParamDecl *CXXABIThisDecl,
                                OpenMPMapClauseKind MapType,
                                bool IsCaptureByLambda, SourceLocation Loc) {
  std::string Name = Var->getNameAsString();
  std::string Strs;
  std::string Capture = IsCaptureByLambda ? "(captured by lambda) " : " ";
  Strs += " \"" + Name + "\"" + Capture + "has an implicit clause: \"map(";
  Strs += MapType == OMPC_MAP_to ? "to : " : "tofrom : ";
  PresumedLoc PLoc = CGF.getContext().getSourceManager().getPresumedLoc(Loc);
  unsigned Line = PLoc.getLine();
  unsigned Column = PLoc.getColumn();
  std::string LineInfo;
  if (IsCaptureByLambda)
    LineInfo = "referenced at line:[" + std::to_string(Line) + ":" +
               std::to_string(Column) + "]";
  else
    LineInfo = "referenced within the construct at line:[" +
               std::to_string(Line) + ":" + std::to_string(Column) + "]";
  if (IsCaptureByLambda)
    return Strs + Name + ")\" because \"" + Name +
           "\" is captured in a lambda mapped on the construct, and is " +
           LineInfo;
  else if (Var == CXXABIThisDecl)
    return Strs + Name + "[:1])\" because \"" + Name + "\" this keyword is " +
           LineInfo;
  else if (isa<FieldDecl>(Var))
    return Strs + Name + ")\" because field \"" + Name +
           "\" is a non-scalar variable " + LineInfo;
  else if (Var->getType()->isPointerType())
    return Strs + Name + "[:0])\" because \"" + Name +
           "\" is a pointer variable " + LineInfo;
  else
    return Strs + Name + ")\" because \"" + Name +
           "\" is a non-scalar variable " + LineInfo;
}

static SourceLocation
getExprLocation(llvm::SmallVector<const Expr *> MapVarExprs,
                const OMPExecutableDirective &Dir, bool *IsCaptureByLambda,
                ImplicitParamDecl *CXXABIThisDecl, const ValueDecl **Var) {
  for (auto *E : MapVarExprs) {
    if (const auto *DRE = dyn_cast<DeclRefExpr>(E))
      if (const auto *VD = dyn_cast<VarDecl>(DRE->getDecl()))
        if (VD == *Var)
          return  E->getExprLoc();
    if (const auto *ME = dyn_cast<MemberExpr>(E))
      if (*Var)
        if (const auto *FD = dyn_cast<FieldDecl>(ME->getMemberDecl()))
          if (const auto *VF = dyn_cast_or_null<FieldDecl>(*Var))
            if (FD == VF)
              return E->getExprLoc();
    if (isa<CXXThisExpr>(E))
      if (!(*Var)) {
        *Var = CXXABIThisDecl;
        return E->getExprLoc();
      }
  }
  const CapturedStmt *CS = Dir.getCapturedStmt(OMPD_target);
  for (CapturedStmt::const_capture_iterator CI = CS->capture_begin(),
                                            CE = CS->capture_end();
       CI != CE; ++CI) {
    if (CI->capturesThis() && !(*Var)) {
      *Var = CXXABIThisDecl;
      return CI->getLocation();
    }
    if (CI->capturesVariable()) {
      // get lambda location
      if (const auto *RD = CI->getCapturedVar()
                               ->getType()
                               .getCanonicalType()
                               .getNonReferenceType()
                               ->getAsCXXRecordDecl())
        if (RD->isLambda())
          for (const LambdaCapture LC : RD->captures())
            if (LC.capturesVariable() && LC.getCapturedVar() == *Var) {
              *IsCaptureByLambda = true;
              return LC.getLocation();
            }
    }
  }
  return SourceLocation();
}

static void
getMapReportInfo(OpenMPLateOutliner &O, const OMPExecutableDirective &Dir,
                 CodeGenFunction &CGF, ImplicitParamDecl *CXXABIThisDecl,
                 SmallVector<CGOpenMPRuntime::LOMapInfo, 4> Info,
                 llvm::MapVector<const VarDecl *, std::string> *FPInfos) {
  const Stmt *S = Dir.getCapturedStmt(OMPD_target)->getCapturedStmt();
  ExprVarRefFinder Finder(CGF, FPInfos);
  Finder.Visit(S);
  llvm::SmallVector<const Expr *> &MapVarExprs = Finder.getMapVarExprs();
  for (auto &I : Info) {
    // Only for implicit map
    if (I.IsImplicit) {
      // For each implicit MapDecl find corresponding first referenced
      // expression in the target region, get Location from Expr then emit
      // remark
      const ValueDecl *Var = I.MapDecl;
      bool IsCaptureByLambda = false;
      SourceLocation Loc = getExprLocation(MapVarExprs, Dir, &IsCaptureByLambda,
                                           CXXABIThisDecl, &Var);
      assert(Loc.isValid() && "Reference location is not set");
      O.emitRemark(getReasonStr(Var, CGF, CXXABIThisDecl, I.MapType,
                                IsCaptureByLambda, Loc));
    }
  }
}

void OpenMPLateOutliner::emitRemark(std::string Str) {
  llvm::OptimizationRemarkEmitter ORE(CGF.CurFn);
  llvm::DiagnosticLocation DL =
      CGF.SourceLocToDebugLoc(Directive.getBeginLoc());
  llvm::OptimizationRemark R("openmp", "Region", DL,
                             &CGF.CurFn->getEntryBlock());
  R << llvm::ore::NV("Construct",
                     getOpenMPDirectiveName(Directive.getDirectiveKind()))
    << " construct:";
  R << Str;
  ORE.emit(R);
}
#endif // INTEL_CUSTOMIZATION

void OpenMPLateOutliner::emitOMPAllDependClauses() {
  if (!CGF.getLangOpts().OpenMPNewDependIR ||
      !Directive.hasClausesOfKind<OMPDependClause>())
    return;

  OMPTaskDataTy Data;
  for (const auto *C : Directive.getClausesOfKind<OMPDependClause>()) {
    auto DepKind = C->getDependencyKind();
    if (DepKind == OMPC_DEPEND_source || DepKind == OMPC_DEPEND_sink) {
      ClauseEmissionHelper CEH(*this, OMPC_depend);
      if (DepKind == OMPC_DEPEND_source)
        addArg("QUAL.OMP.DEPEND.SOURCE");
      else
        addArg("QUAL.OMP.DEPEND.SINK");
      for (unsigned I = 0, E = C->getNumLoops(); I < E; ++I)
        addArg(CGF.EmitScalarExpr(C->getLoopData(I)));
      continue;
    }
    OMPTaskDataTy::DependData &DD =
        Data.Dependences.emplace_back(DepKind, C->getModifier());
    DD.DepExprs.append(C->varlist_begin(), C->varlist_end());
  }
  if (Data.Dependences.size() == 0)
    return;

  Address DependenciesArray = Address::invalid();
  llvm::Value *NumOfElements;
  std::tie(NumOfElements, DependenciesArray) =
      CGF.CGM.getOpenMPRuntime().emitDependClause(CGF, Data.Dependences,
                                                  Directive.getBeginLoc());
  ClauseEmissionHelper CEH(*this, OMPC_unknown);
  addArg("QUAL.OMP.DEPARRAY");
  addArg(NumOfElements);
  addArg(DependenciesArray.getPointer());
}

void OpenMPLateOutliner::emitOMPAllMapClauses() {
  for (const auto *C : Directive.getClausesOfKind<OMPMapClause>()) {
    for (const auto *E : C->varlists()) {
      // When there is a map-chain in the IR for the the map clause, the
      // computations for various expressions used in the map-chain are to be
      // emitted after the implicit task, and before the target directive
      if (const VarDecl *ExplicitVar = getExplicitVarDecl(E)) {
        if (isImplicitTask(OMPD_task)) {
          ImplicitMap.insert(std::make_pair(ExplicitVar, ICK_shared));
          continue;
        } else {
          addExplicit(ExplicitVar, OMPC_map);
        }
      }
    }
  }
  for (const auto *C : Directive.getClausesOfKind<OMPIsDevicePtrClause>()) {
    for (const auto *E : C->varlists()) {
      const VarDecl *VD = getExplicitVarDecl(E);
      assert(VD && "expected VarDecl in is_device_ptr clause");
      addExplicit(VD, OMPC_is_device_ptr);
    }
  }
  SmallVector<CGOpenMPRuntime::LOMapInfo, 4> Info;
  {
    // Generate map values and emit outside the current directive.
    ClauseEmissionHelper CEH(*this, OMPC_map, /*InitStr=*/"",
                             /*EmitClause=*/false);
    CGOpenMPRuntime::getLOMapInfo(Directive, CGF, &Info);
  }
  for (const auto &I : Info) {
    OpenMPClauseKind CK = OMPC_map;
    if (CurrentDirectiveKind == OMPD_target_update)
      CK = I.MapType == OMPC_MAP_to ? OMPC_to : OMPC_from;
    llvm::Value *MapperFn = nullptr;
    if (I.Mapper)
      MapperFn = CGF.CGM.getOpenMPRuntime().getOrCreateUserDefinedMapperFunc(
          cast<OMPDeclareMapperDecl>(I.Mapper));
    if (!I.IsChain && I.Var) {
      QualType Ty = I.Var->getType();
      if (isImplicitTask(OMPD_task) && !isExplicitForIsDevicePtr(I.Var)) {
        // OpenMP 5.1 target Construct:
        // If a variable or part of a variable is mapped by the target
        // construct and does not appear as a list item in an
        // in_reduction clause on the construct, the variable has a
        // default data-sharing attribute of shared in the data
        // environment of the target task.
        // Note: no need to check in_reduction, since it is alread added
        // as explicit clause and implicit map gets skipped.
        if (isImplicit(I.Var))
          ImplicitMap.erase(ImplicitMap.find(I.Var));
        // For whole variable map eliminate implicit map with pointer type.
        // For part of varable map include varible with array or record
        // type.
        if ((I.Base == I.Pointer &&
             !(I.IsImplicit && I.Var->getType()->isPointerType())) ||
            I.Base != I.Pointer && (I.Var->getType()->isArrayType() ||
                                    I.Var->getType()->isRecordType()))
          ImplicitMap.insert(std::make_pair(I.Var, ICK_shared));
      } else
        addExplicit(I.Var, OMPC_map);
      if (CurrentDirectiveKind == OMPD_target)
        if ((Ty->isReferenceType() || Ty->isAnyPointerType()) &&
            isa<llvm::LoadInst>(I.Base)) {
          MapTemps.emplace_back(I.Base, I.Var);
          if (isImplicit(I.Var) && Ty->isReferenceType() &&
              Ty.getNonReferenceType()->isScalarType() &&
              !Ty.getNonReferenceType()->isPointerType()) {
            // Emit implicit clause instead map clause for
            // variable with reference type to non-pointer scalar.
            QualType VTy = I.Var->getType().getNonReferenceType();
            emitImplicit(I.Base, CGF.ConvertTypeForMem(VTy),
                         ImplicitMap[I.Var]);
            continue;
          }
        }
    }
    ClauseEmissionHelper CEH(*this, CK);
    ClauseStringBuilder &CSB = CEH.getBuilder();
    buildMapQualifier(CSB, I.MapType, I.Modifiers, I.Var);
    if (!I.IsChain && I.Var)
      if (I.Var->getType()->isAnyPointerType() &&
          I.Var->getType()->getPointeeType()->isFunctionType())
        CSB.setFptr();
    if (I.IsChain)
      CSB.setChain();
    addArg(CSB.getString());
    for (auto *V :
         {I.Base, I.Pointer, I.Size, I.Type, I.OffloadName, MapperFn}) {
      if (!V)
        V =  llvm::ConstantPointerNull::get(CGF.VoidPtrTy);
      addArg(V);
    }
  }
#if INTEL_CUSTOMIZATION
  // generate info for map report.
  if (isOpenMPTargetExecutionDirective(Directive.getDirectiveKind()) &&
      (!CGF.CGM.getCodeGenOpts().OptRecordFile.empty() ||
       CGF.CGM.getCodeGenOpts().OptimizationRemark.patternMatches("openmp"))) {
    llvm::MapVector<const VarDecl *, std::string> FPInfos;
    getMapReportInfo(*this, Directive, CGF, CGF.getCXXABIThisDecl(), Info,
                     &FPInfos);
    OptRepFPMapInfos = std::move(FPInfos);
  }
#endif // INTEL_CUSTOMIZATION
}

#if INTEL_CUSTOMIZATION
void OpenMPLateOutliner::emitOMPOmpxAssertClause(const OMPOmpxAssertClause *) {
  // This is handled with the LoopHint mechanism.
}

void OpenMPLateOutliner::emitOMPTileClause(const OMPTileClause *C) {
  ClauseEmissionHelper CEH(*this, OMPC_tile);
  addArg("QUAL.OMP.TILE");
  for (auto *E : C->sizes())
    addArg(CGF.EmitScalarExpr(E));
}

void OpenMPLateOutliner::emitOMPInteropClause(const OMPInteropClause *C) {
  ClauseEmissionHelper CEH(*this, OMPC_interop, "QUAL.OMP.INTEROP");
  ClauseStringBuilder &CSB = CEH.getBuilder();
  addArg(CSB.getString());
  for (auto *E : C->varlists())
    addArg(CGF.EmitScalarExpr(E));
}

void OpenMPLateOutliner::emitOMPOmpxOverlapClause(
    const OMPOmpxOverlapClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_ompx_overlap, "QUAL.OMP.OVERLAP");
  ClauseStringBuilder &CSB = CEH.getBuilder();
  addArg(CSB.getString());
  addArg(CGF.EmitScalarExpr(Cl->getOverlap()));
}
void OpenMPLateOutliner::emitOMPOmpxMonotonicClause(
    const OMPOmpxMonotonicClause *Cl) {
  for (auto *E : Cl->varlists()) {
    ClauseEmissionHelper CEH(*this, OMPC_ompx_monotonic,
                             "QUAL.OMP.MONOTONIC");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    E = E->IgnoreParenImpCasts();
    if (E->getType()->isPointerType())
      CSB.setPtrToPtr();
    addArg(CSB.getString());
    addArg(E);
    addArg(Cl->getStep() ? CGF.EmitScalarExpr(Cl->getStep())
                         : CGF.Builder.getInt32(0));
  }
}
#endif // INTEL_CUSTOMIZATION

void OpenMPLateOutliner::emitOMPBindClause(const OMPBindClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_bind);
  switch (Cl->getBindKind()) {
  case OMPC_BIND_teams:
    addArg("QUAL.OMP.BIND.TEAMS");
    break;
  case OMPC_BIND_parallel:
    addArg("QUAL.OMP.BIND.PARALLEL");
    break;
  case OMPC_BIND_thread:
    addArg("QUAL.OMP.BIND.THREAD");
    break;
  case OMPC_BIND_unknown:
    llvm_unreachable("Unknown bind clause");
  }
}

void OpenMPLateOutliner::emitOMPThreadsClause(const OMPThreadsClause *) {
  ClauseEmissionHelper CEH(*this, OMPC_threads);
  addArg("QUAL.OMP.ORDERED.THREADS");
}

void OpenMPLateOutliner::emitOMPSIMDClause(const OMPSIMDClause *) {
  ClauseEmissionHelper CEH(*this, OMPC_simd);
  addArg("QUAL.OMP.ORDERED.SIMD");
}

void OpenMPLateOutliner::emitOMPAllocateClause(const OMPAllocateClause *Cl) {
  llvm::Value *Allocator = nullptr;
  // Allocator must always be i64.
  if (auto *A = Cl->getAllocator()) {
    A = A->IgnoreImpCasts();
    Allocator = CGF.Builder.CreateIntCast(CGF.EmitScalarExpr(A), CGF.Int64Ty,
                                          /*isSigned=*/false);
  }

  unsigned UserAlign = 0;
  if (auto *Align = Cl->getAlignment())
    UserAlign = Align->EvaluateKnownConstInt(CGF.getContext()).getExtValue();

  // OpenMP5.1 pg 185 lines 7-10
  //   Each item in the align modifier list must be aligned to the maximum
  //   of the specified alignment and the type's natural alignment.
  //
  // For IR consistency, if no alignment specified then use the natural
  // alignment.
  auto ChooseAlignValue = [this] (QualType ListItemTy, unsigned UserAlign) {
    CharUnits NaturalAlign = CGF.CGM.getNaturalTypeAlignment(ListItemTy);
    if (!UserAlign || UserAlign <= NaturalAlign.getQuantity())
      return llvm::ConstantInt::get(CGF.Int64Ty, NaturalAlign.getQuantity());
    else
      return llvm::ConstantInt::get(CGF.Int64Ty, UserAlign);
  };

  for (const auto *E : Cl->varlists()) {
    ClauseEmissionHelper CEH(*this, OMPC_allocate);
    addArg("QUAL.OMP.ALLOCATE");
    addArg(ChooseAlignValue(E->getType(), UserAlign));
    addArg(E);
    if (Allocator)
      addArg(Allocator);
  }
}

void OpenMPLateOutliner::emitOMPOrderClause(const OMPOrderClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_order);
  switch (Cl->getKind()) {
  case OMPC_ORDER_concurrent:
    addArg("QUAL.OMP.ORDER.CONCURRENT");
    break;
  case OMPC_ORDER_unknown:
    llvm_unreachable("Unknown order clause");
  }
}

void OpenMPLateOutliner::emitOMPUseDeviceAddrClause(
    const OMPUseDeviceAddrClause *Cl) {
  for (auto *E : Cl->varlists()) {
    const VarDecl *VD = getExplicitVarDecl(E);
    assert(VD && "expected VarDecl in use_device_addr clause");
    addExplicit(VD, OMPC_use_device_addr);
    bool IsCapturedExpr = isa<OMPCapturedExprDecl>(VD);
    bool IsRef = !IsCapturedExpr && VD->getType()->isReferenceType();
    ClauseEmissionHelper CEH(*this, OMPC_use_device_addr,
                             "QUAL.OMP.USE_DEVICE_ADDR");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    if (isa<ArraySubscriptExpr>(E->IgnoreParenImpCasts()) ||
        E->getType()->isSpecificPlaceholderType(BuiltinType::OMPArraySection))
      CSB.setArrSect();
    if (IsRef)
      CSB.setByRef();
    addArg(CSB.getString());
    addArg(E, IsRef);
  }
}

void OpenMPLateOutliner::emitOMPHasDeviceAddrClause(
    const OMPHasDeviceAddrClause *Cl) {
  for (auto *E : Cl->varlists()) {
    const VarDecl *VD = getExplicitVarDecl(E);
    assert(VD && "expected VarDecl in has_device_addr clause");
    addExplicit(VD, OMPC_has_device_addr);
    bool IsCapturedExpr = isa<OMPCapturedExprDecl>(VD);
    bool IsRef = !IsCapturedExpr && VD->getType()->isReferenceType();
    ClauseEmissionHelper CEH(*this, OMPC_has_device_addr,
                             "QUAL.OMP.HAS_DEVICE_ADDR");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    if (isa<ArraySubscriptExpr>(E->IgnoreParenImpCasts()) ||
        E->getType()->isSpecificPlaceholderType(BuiltinType::OMPArraySection))
      CSB.setArrSect();
    if (IsRef)
      CSB.setByRef();
    addArg(CSB.getString());
    addArg(E, IsRef);
  }
}

void OpenMPLateOutliner::emitOMPNontemporalClause(
    const OMPNontemporalClause *Cl) {
  for (auto *E : Cl->varlists()) {
    ClauseEmissionHelper CEH(*this, OMPC_nontemporal, "QUAL.OMP.NONTEMPORAL");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    E = E->IgnoreParenImpCasts();
    if (E->getType()->isPointerType())
      CSB.setPtrToPtr();
    addArg(CSB.getString());
    addArg(E);
  }
}

void OpenMPLateOutliner::emitOMPNovariantsClause(
    const OMPNovariantsClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_novariants);
  addArg("QUAL.OMP.NOVARIANTS");
  addArg(CGF.EvaluateExprAsBool(Cl->getCondition()));
}

void OpenMPLateOutliner::emitOMPNocontextClause(const OMPNocontextClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_nocontext);
  addArg("QUAL.OMP.NOCONTEXT");
  addArg(CGF.EvaluateExprAsBool(Cl->getCondition()));
}

void OpenMPLateOutliner::emitOMPDataClause(const OMPDataClause *C) {
  for (auto *E : C->varlists()) {
    ClauseEmissionHelper CEH(*this, OMPC_data, "QUAL.OMP.DATA");
    ClauseStringBuilder &CSB = CEH.getBuilder();

    llvm::Type *ElemTy = nullptr;
    if (isa<ArraySubscriptExpr>(E->IgnoreParenImpCasts()) ||
        E->getType()->isSpecificPlaceholderType(BuiltinType::OMPArraySection)) {
      QualType BaseTy = getArraySectionBase(E)->getType();
      if (BaseTy->isPointerType())
        ElemTy = CGF.ConvertTypeForMem(BaseTy->getPointeeType());
    }

    addArg(CSB.getString());
    addArg(E, /*IsRef=*/false, /*IsTyped=*/true, /*NeedsTypedElements=*/true,
           ElemTy, /*ArraySecUsesBase=*/false);

    if (const Expr *Hint = C->getHint())
      addArg(CGF.Builder.CreateIntCast(CGF.EmitScalarExpr(Hint), CGF.Int32Ty,
                                       /*isSigned=*/false));
    else
      addArg(CGF.Builder.getInt32(0));
  }
}

void OpenMPLateOutliner::emitOMPInclusiveClause(const OMPInclusiveClause *Cl) {
  for (auto *E : Cl->varlists()) {
    const VarDecl *VD = getExplicitVarDecl(E);
    assert(VD && "expected VarDecl in inclusive clause");
    addExplicit(VD, OMPC_inclusive);
    ClauseEmissionHelper CEH(*this, OMPC_inclusive, "QUAL.OMP.INCLUSIVE");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    if (UseTypedClauses)
      CSB.setTyped();
    addArg(CSB.getString());
    addTypedArg(getExplicitDeclRefOrNull(E));
    addArg(llvm::ConstantInt::get(CGF.SizeTy, CGF.getInscanVarIndex(VD)));
  }
}

void OpenMPLateOutliner::emitOMPExclusiveClause(const OMPExclusiveClause *Cl) {
  for (auto *E : Cl->varlists()) {
    const VarDecl *VD = getExplicitVarDecl(E);
    assert(VD && "expected VarDecl in exclusive clause");
    addExplicit(VD, OMPC_exclusive);
    ClauseEmissionHelper CEH(*this, OMPC_exclusive, "QUAL.OMP.EXCLUSIVE");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    if (UseTypedClauses)
      CSB.setTyped();
    addArg(CSB.getString());
    addTypedArg(getExplicitDeclRefOrNull(E));
    addArg(llvm::ConstantInt::get(CGF.SizeTy, CGF.getInscanVarIndex(VD)));
  }
}

void OpenMPLateOutliner::emitOMPReadClause(const OMPReadClause *) {}
void OpenMPLateOutliner::emitOMPWriteClause(const OMPWriteClause *) {}
void OpenMPLateOutliner::emitOMPFromClause(const OMPFromClause *) {assert(false);}
void OpenMPLateOutliner::emitOMPToClause(const OMPToClause *) {assert(false);}
void OpenMPLateOutliner::emitOMPMapClause(const OMPMapClause *) {assert(false);}
void OpenMPLateOutliner::emitOMPUpdateClause(const OMPUpdateClause *) {}
void OpenMPLateOutliner::emitOMPCaptureClause(const OMPCaptureClause *) {}
void OpenMPLateOutliner::emitOMPCompareClause(const OMPCompareClause *) {}
void OpenMPLateOutliner::emitOMPSeqCstClause(const OMPSeqCstClause *) {}
void OpenMPLateOutliner::emitOMPUnifiedAddressClause(
    const OMPUnifiedAddressClause *) {}
void OpenMPLateOutliner::emitOMPUnifiedSharedMemoryClause(
    const OMPUnifiedSharedMemoryClause *) {}
void OpenMPLateOutliner::emitOMPReverseOffloadClause(
    const OMPReverseOffloadClause *) {}
void OpenMPLateOutliner::emitOMPDynamicAllocatorsClause(
    const OMPDynamicAllocatorsClause *) {}
void OpenMPLateOutliner::emitOMPAtomicDefaultMemOrderClause(
    const OMPAtomicDefaultMemOrderClause *) {}
void OpenMPLateOutliner::emitOMPAllocatorClause(const OMPAllocatorClause *) {}
void OpenMPLateOutliner::emitOMPAcqRelClause(const OMPAcqRelClause *) {}
void OpenMPLateOutliner::emitOMPAcquireClause(const OMPAcquireClause *) {}
void OpenMPLateOutliner::emitOMPReleaseClause(const OMPReleaseClause *) {}
void OpenMPLateOutliner::emitOMPRelaxedClause(const OMPRelaxedClause *) {}
void OpenMPLateOutliner::emitOMPDepobjClause(const OMPDepobjClause *) {
  assert(false);
}
void OpenMPLateOutliner::emitOMPDetachClause(const OMPDetachClause *) {}
void OpenMPLateOutliner::emitOMPUsesAllocatorsClause(
    const OMPUsesAllocatorsClause *) {}
void OpenMPLateOutliner::emitOMPAffinityClause(const OMPAffinityClause *) {}
void OpenMPLateOutliner::emitOMPSizesClause(const OMPSizesClause *) {}
void OpenMPLateOutliner::emitOMPAlignClause(const OMPAlignClause *Cl) {}
void OpenMPLateOutliner::emitOMPFullClause(const OMPFullClause *Cl) {}
void OpenMPLateOutliner::emitOMPPartialClause(const OMPPartialClause *Cl) {}

void OpenMPLateOutliner::emitOMPInitClause(const OMPInitClause *Cl) {
  const VarDecl *VD = getExplicitVarDecl(Cl->getInteropVar());
  assert(VD && "expected VarDecl in init clause");
  addExplicit(VD, OMPC_init);

  // Gather any valid preferences first.
  SmallVector<llvm::Value *, 3> PrefValues;
  for (const Expr *PE : Cl->prefs())
    if (unsigned PrefValue = CGF.CGM.getValidInteropPreferTypeValue(PE))
      PrefValues.push_back(CGF.Builder.getInt64(PrefValue));

  ClauseEmissionHelper CEH(*this, OMPC_init, "QUAL.OMP.INIT");
  ClauseStringBuilder &CSB = CEH.getBuilder();
  if (Cl->getIsTarget())
    CSB.setTarget();
  if (Cl->getIsTargetSync())
    CSB.setTargetSync();
  if (!PrefValues.empty())
    CSB.setPrefer();
  addArg(CSB.getString());
  addArg(Cl->getInteropVar());
  for (auto *V : PrefValues)
    addArg(V);
}

void OpenMPLateOutliner::emitOMPUseClause(const OMPUseClause *Cl) {
  const VarDecl *VD = getExplicitVarDecl(Cl->getInteropVar());
  assert(VD && "expected VarDecl in use clause");
  addExplicit(VD, OMPC_use);
  ClauseEmissionHelper CEH(*this, OMPC_use, "QUAL.OMP.USE");
  addArg(CEH.getBuilder().getString());
  addArg(Cl->getInteropVar());
}

void OpenMPLateOutliner::emitOMPDestroyClause(const OMPDestroyClause *Cl) {
  if (const VarDecl *VD = getExplicitVarDecl(Cl->getInteropVar())) {
    assert(VD && "expected VarDecl in use clause");
    addExplicit(VD, OMPC_destroy);
    ClauseEmissionHelper CEH(*this, OMPC_use, "QUAL.OMP.DESTROY");
    addArg(CEH.getBuilder().getString());
    addArg(Cl->getInteropVar());
  }
}

void OpenMPLateOutliner::addFenceCalls(bool IsBegin) {
  // Check current specific directive rather than directive kind (it can
  // potentially be a combined directive, and those are broken up into
  // specific directives).
  switch (CurrentDirectiveKind) {
  case OMPD_atomic:
  case OMPD_critical:
  case OMPD_single:
  case OMPD_master:
  case OMPD_masked:
    if (IsBegin)
      CGF.Builder.CreateFence(llvm::AtomicOrdering::Acquire);
    else
      CGF.Builder.CreateFence(llvm::AtomicOrdering::Release);
    break;
  case OMPD_barrier:
  case OMPD_taskwait:
  case OMPD_scan:
    if (IsBegin)
      CGF.Builder.CreateFence(llvm::AtomicOrdering::AcquireRelease);
    break;
  default:
    break;
  }
}

void OpenMPLateOutliner::HandleImplicitVar(const Expr *E,
                                           ImplicitClauseKind ICK) {
  auto VD = cast<VarDecl>(cast<DeclRefExpr>(E)->getDecl());
  ImplicitMap.insert(std::make_pair(VD, ICK));
}

/// If in a target or teams construct and there is a loop inside
/// without any other statements between, return it.
static const OMPLoopDirective *
GetCloselyNestedLoop(const OMPExecutableDirective &S) {
  OpenMPDirectiveKind Kind = S.getDirectiveKind();

  if (isOpenMPLoopDirective(Kind) && isOpenMPTargetExecutionDirective(Kind))
    return cast<OMPLoopDirective>(&S);

  if (isOpenMPLoopDirective(Kind) && isOpenMPTeamsDirective(Kind))
    return cast<OMPLoopDirective>(&S);

  const Stmt *CS = nullptr;
  if (Kind == OMPD_target || Kind == OMPD_teams || Kind == OMPD_target_teams) {
    if ((CS = S.getInnermostCapturedStmt()->getCapturedStmt())) {
      if (const auto *CompStmt = dyn_cast<CompoundStmt>(CS)) {
        if (CompStmt->body_front() &&
            CompStmt->body_front() == CompStmt->body_back())
          CS = CompStmt->body_front();
      }
    }
    if (CS && Kind == OMPD_target) {
      if (auto *TeamsDir = dyn_cast<OMPTeamsDirective>(CS)) {
        if ((CS = TeamsDir->getInnermostCapturedStmt()->getCapturedStmt())) {
          if (const auto *CompStmt = dyn_cast<CompoundStmt>(CS)) {
            if (CompStmt->body_front() &&
                CompStmt->body_front() == CompStmt->body_back())
              CS = CompStmt->body_front();
          }
        }
      }
    }
    if (auto *LD = dyn_cast_or_null<OMPLoopDirective>(CS))
      return LD;
  }
  return nullptr;
}

OpenMPLateOutliner::OpenMPLateOutliner(CodeGenFunction &CGF,
                                       const OMPExecutableDirective &D,
                                       OpenMPDirectiveKind Kind)
    : CGF(CGF), C(CGF.CGM.getLLVMContext()), Directive(D),
      CurrentDirectiveKind(Kind) {
  // Set an attribute indicating that the routine may have OpenMP directives
  // (represented with llvm intrinsics) in the LLVM IR
  CGF.CurFn->addFnAttr("may-have-openmp-directive", "true");

  if (CurrentDirectiveKind == OMPD_unknown)
    CurrentDirectiveKind = D.getDirectiveKind();

  if (D.hasAssociatedStmt() && needsVLAExprEmission()) {
    const auto *CS = cast_or_null<CapturedStmt>(D.getAssociatedStmt());
    CGF.VLASizeMapHandler->ModifyVLASizeMap(CS);
  }

  RegionEntryDirective =
      CGF.CGM.getIntrinsic(llvm::Intrinsic::directive_region_entry);
  RegionExitDirective =
      CGF.CGM.getIntrinsic(llvm::Intrinsic::directive_region_exit);

  UseTypedClauses = CGF.CGM.getCodeGenOpts().OpenMPTypedClauses;

  if (isOpenMPLoopDirective(CurrentDirectiveKind)) {
    auto *LoopDir = dyn_cast<OMPLoopDirective>(&D);

    // If this loop was transformed by a tile directive, find the original
    // loop variables and mark them private.
    if (const auto *CS = dyn_cast<CapturedStmt>(LoopDir->getAssociatedStmt()))
      if (const auto *TileDir =
              dyn_cast<OMPTileDirective>(CS->getCapturedStmt()))
        if (const auto *DS = dyn_cast<DeclStmt>(TileDir->getPreInits()))
          for (const Decl *D : DS->decls())
            if (const auto *VD = dyn_cast<VarDecl>(D))
              ImplicitMap.insert(std::make_pair(VD, ICK_private));

    for (auto *E : LoopDir->counters()) {
      auto *VD = cast<VarDecl>(cast<DeclRefExpr>(E)->getDecl());
      if (isOpenMPSimdDirective(LoopDir->getDirectiveKind())) {
        if (LoopDir->getLateOutlineLinearCounterIncrement()) {
          if (CGF.IsPrivateCounter(VD))
            ImplicitMap.insert(std::make_pair(VD, ICK_linear_private));
          else
            ImplicitMap.insert(std::make_pair(VD, ICK_linear_lastprivate));
        } else {
          if (!CGF.IsPrivateCounter(VD))
            ImplicitMap.insert(std::make_pair(VD, ICK_lastprivate));
        }
      } else
        ImplicitMap.insert(std::make_pair(VD, ICK_private));
    }
    for (const auto *C : LoopDir->getClausesOfKind<OMPOrderedClause>()) {
      if (!C->getNumForLoops())
        continue;
      for (unsigned I = LoopDir->getLoopsNumber(),
                    E = C->getLoopNumIterations().size();
           I < E; ++I) {
        const auto *DRE = cast<DeclRefExpr>(C->getLoopCounter(I));
        const auto *VD = cast<VarDecl>(DRE->getDecl());
        if (isOpenMPSimdDirective(CurrentDirectiveKind))
          ImplicitMap.insert(std::make_pair(VD, ICK_unknown));
        else
          ImplicitMap.insert(std::make_pair(VD, ICK_private));
      }
    }
#if INTEL_CUSTOMIZATION
    if (CGF.useUncollapsedLoop(*LoopDir)) {
      auto UncollapsedIVs = LoopDir->uncollapsedIVs();
      auto UncollapsedLowerBounds = LoopDir->uncollapsedLowerBounds();
      auto UncollapsedUpperBounds = LoopDir->uncollapsedUpperBounds();
      for (unsigned I = 0, E = LoopDir->getLoopsNumber(); I < E; ++I) {
        HandleImplicitVar(UncollapsedIVs[I], ICK_normalized_iv);
        HandleImplicitVar(UncollapsedUpperBounds[I], ICK_normalized_ub);
        if (isOpenMPWorksharingDirective(CurrentDirectiveKind) ||
            isOpenMPGenericLoopDirective(CurrentDirectiveKind) ||
            isOpenMPTaskLoopDirective(CurrentDirectiveKind) ||
            isOpenMPDistributeDirective(CurrentDirectiveKind)) {
          HandleImplicitVar(UncollapsedLowerBounds[I], ICK_firstprivate);
        }
      }
      return;
    }
#endif // INTEL_CUSTOMIZATION

    HandleImplicitVar(LoopDir->getIterationVariable(), ICK_normalized_iv);
    HandleImplicitVar(LoopDir->getUpperBoundVariable(), ICK_normalized_ub);
    if (isOpenMPWorksharingDirective(CurrentDirectiveKind) ||
        isOpenMPGenericLoopDirective(CurrentDirectiveKind) ||
        isOpenMPTaskLoopDirective(CurrentDirectiveKind) ||
        isOpenMPDistributeDirective(CurrentDirectiveKind)) {
      HandleImplicitVar(LoopDir->getLowerBoundVariable(), ICK_firstprivate);
    }
  } else {
    // CurrentDirectiveKind is not a loop, but there could be a nested loop.
    // Add implicit private clauses to the surrounding target/teams.
    if (const OMPLoopDirective *LD = GetCloselyNestedLoop(D)) {
      for (auto *E : LD->counters()) {
        auto *VD = cast<VarDecl>(cast<DeclRefExpr>(E)->getDecl());
        ImplicitMap.insert(std::make_pair(VD, ICK_private));
      }
      for (const auto *C : LD->getClausesOfKind<OMPPrivateClause>()) {
        for (const auto *Ref : C->varlists()) {
          if (const auto *DRE = dyn_cast<DeclRefExpr>(Ref)) {
            auto *VD = cast<VarDecl>(DRE->getDecl());
            ImplicitMap.insert(std::make_pair(VD, ICK_private));
          }
        }
      }
    }
  }
}

OpenMPLateOutliner::~OpenMPLateOutliner() {
  // Clauses need values from outside the region, so restore here before
  // emitting clauses.
  CGF.VLASizeMapHandler->RestoreVLASizeMap();
  addImplicitClauses();

  // Insert the start directives.
  auto EndIP = CGF.Builder.saveIP();
  setInsertPoint();

  for (auto &D : Directives) {
    llvm::CallInst *MarkerCall = D.CallEntry;
    CGF.Builder.SetInsertPoint(MarkerCall);
    D.CallEntry = CGF.Builder.CreateCall(RegionEntryDirective, {}, D.OpBundles);
    if (MarkerCall != MarkerInstruction)
      MarkerCall->eraseFromParent();
    D.clear();
    // Place the end directive in place of the start.
    setInsertPoint();
    emitDirective(D, D.End);
  }

  addFenceCalls(/*IsBegin=*/true);
  CGF.Builder.restoreIP(EndIP);
  addFenceCalls(/*IsBegin=*/false);

  // Insert the end directives.
  if (!CGF.HaveInsertPoint()) {
    // This is to handle unconditional jump(backward goto), return and exit
    llvm::BasicBlock *Dummy = CGF.createBasicBlock("dummy");
    CGF.EmitBlock(Dummy);
    CGF.Builder.SetInsertPoint(Dummy);
  }
  for (auto I = Directives.rbegin(), E = Directives.rend(); I != E; ++I)
    CGF.Builder.CreateCall(RegionExitDirective, {I->CallEntry}, I->OpBundles);
  MarkerInstruction->eraseFromParent();

  addRefsToOuter();
  if (CurrentDirectiveKind == OMPD_target)
    CGF.clearMappedTemps();
  if (CurrentDirectiveKind == OMPD_simd)
    for (const auto *C : Directive.getClausesOfKind<OMPReductionClause>())
      if (C->getModifier() == OMPC_REDUCTION_inscan) {
        CGF.clearInscanVars();
        break;
      }
}

void OpenMPLateOutliner::emitOMPParallelDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.PARALLEL", "DIR.OMP.END.PARALLEL",
                             OMPD_parallel);
}
void OpenMPLateOutliner::emitOMPParallelForDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.PARALLEL.LOOP",
                             "DIR.OMP.END.PARALLEL.LOOP", OMPD_parallel_for);
}
void OpenMPLateOutliner::emitOMPSIMDDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.SIMD", "DIR.OMP.END.SIMD", OMPD_simd);
}

void OpenMPLateOutliner::emitOMPForDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.LOOP", "DIR.OMP.END.LOOP", OMPD_for);
}

void OpenMPLateOutliner::emitOMPForSimdDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.LOOP", "DIR.OMP.END.LOOP", OMPD_for);
  startDirectiveIntrinsicSet("DIR.OMP.SIMD", "DIR.OMP.END.SIMD", OMPD_simd);
}

void OpenMPLateOutliner::emitOMPParallelForSimdDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.PARALLEL.LOOP",
                             "DIR.OMP.END.PARALLEL.LOOP", OMPD_parallel_for);
  startDirectiveIntrinsicSet("DIR.OMP.SIMD", "DIR.OMP.END.SIMD", OMPD_simd);
}

void OpenMPLateOutliner::emitOMPTaskLoopDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.TASKLOOP", "DIR.OMP.END.TASKLOOP",
                             OMPD_taskloop);
}

void OpenMPLateOutliner::emitOMPTaskLoopSimdDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.TASKLOOP", "DIR.OMP.END.TASKLOOP",
                             OMPD_taskloop);
  startDirectiveIntrinsicSet("DIR.OMP.SIMD", "DIR.OMP.END.SIMD", OMPD_simd);
}

void OpenMPLateOutliner::emitOMPAtomicDirective(OMPAtomicClause ClauseKind) {
  startDirectiveIntrinsicSet("DIR.OMP.ATOMIC", "DIR.OMP.END.ATOMIC",
                             OMPD_atomic);

  StringRef Op = "QUAL.OMP.UPDATE";
  switch (ClauseKind) {
  case OMP_read:
    Op = "QUAL.OMP.READ";
    break;
  case OMP_write:
    Op = "QUAL.OMP.WRITE";
    break;
  case OMP_update:
    break;
  case OMP_capture:
    Op = "QUAL.OMP.CAPTURE";
    break;
  case OMP_read_seq_cst:
    Op = "QUAL.OMP.READ.SEQ_CST";
    break;
  case OMP_write_seq_cst:
    Op = "QUAL.OMP.WRITE.SEQ_CST";
    break;
  case OMP_update_seq_cst:
    Op = "QUAL.OMP.UPDATE.SEQ_CST";
    break;
  case OMP_capture_seq_cst:
    Op = "QUAL.OMP.CAPTURE.SEQ_CST";
    break;
  }
  ClauseEmissionHelper CEH(*this, OMPC_unknown);
  addArg(Op);
}
void OpenMPLateOutliner::emitOMPSingleDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.SINGLE", "DIR.OMP.END.SINGLE",
                             OMPD_single);
}
void OpenMPLateOutliner::emitOMPMasterDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.MASTER", "DIR.OMP.END.MASTER",
                             OMPD_master);
}
void OpenMPLateOutliner::emitOMPMaskedDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.MASKED", "DIR.OMP.END.MASKED",
                             OMPD_masked);
}
void OpenMPLateOutliner::emitOMPCriticalDirective(const StringRef Name) {
  startDirectiveIntrinsicSet("DIR.OMP.CRITICAL", "DIR.OMP.END.CRITICAL",
                             OMPD_critical);
  if (!Name.empty()) {
    ClauseEmissionHelper CEH(*this, OMPC_unknown);
    addArg("QUAL.OMP.NAME");
    addArg(llvm::ConstantDataArray::getString(C, Name, /*AddNull=*/false));
  }
}
void OpenMPLateOutliner::emitOMPOrderedDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.ORDERED", "DIR.OMP.END.ORDERED",
                             OMPD_ordered);
}
void OpenMPLateOutliner::emitOMPScanDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.SCAN", "DIR.OMP.END.SCAN",
                             OMPD_scan);
}
void OpenMPLateOutliner::emitOMPTargetDirective(int OffloadEntryIndex) {
  startDirectiveIntrinsicSet("DIR.OMP.TARGET", "DIR.OMP.END.TARGET",
                             OMPD_target);
  {
    // Add operand bundle for the offload entry index.
    ClauseEmissionHelper CEH(*this, OMPC_unknown);
    addArg("QUAL.OMP.OFFLOAD.ENTRY.IDX");
    addArg(CGF.Builder.getInt32(OffloadEntryIndex));
  }
  for (const auto *Cl : Directive.getClausesOfKind<OMPIsDevicePtrClause>()) {
    for (const auto *E : Cl->varlists()) {
      ClauseEmissionHelper CEH(*this, OMPC_unknown);
      addArg("QUAL.OMP.LIVEIN");
      addArg(E);
    }
  }
}
void OpenMPLateOutliner::emitOMPTargetDataDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.TARGET.DATA", "DIR.OMP.END.TARGET.DATA",
                             OMPD_target_data);
}
void OpenMPLateOutliner::emitOMPTargetUpdateDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.TARGET.UPDATE",
                             "DIR.OMP.END.TARGET.UPDATE", OMPD_target_update);
}
void OpenMPLateOutliner::emitOMPTargetEnterDataDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.TARGET.ENTER.DATA",
                             "DIR.OMP.END.TARGET.ENTER.DATA",
                             OMPD_target_enter_data);
}
void OpenMPLateOutliner::emitOMPTargetExitDataDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.TARGET.EXIT.DATA",
                             "DIR.OMP.END.TARGET.EXIT.DATA",
                             OMPD_target_exit_data);
}
void OpenMPLateOutliner::emitOMPTaskDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.TASK", "DIR.OMP.END.TASK", OMPD_task);
  if (CodeGenFunction::requiresImplicitTask(Directive)) {
    bool NeedIf = Directive.hasClausesOfKind<OMPDependClause>() ||
                  Directive.hasClausesOfKind<OMPInReductionClause>();
    NeedIf = NeedIf && !Directive.hasClausesOfKind<OMPNowaitClause>();
    if (NeedIf) {
      ClauseEmissionHelper CEH(*this, OMPC_if);
      addArg("QUAL.OMP.IF");
      addArg(CGF.Builder.getInt32(0));
    }
    ClauseEmissionHelper CEH(*this, OMPC_unknown);
    if (Directive.getDirectiveKind() == OMPD_dispatch)
      addArg("QUAL.OMP.IMPLICIT");
    else
      addArg("QUAL.OMP.TARGET.TASK");
  }
}
void OpenMPLateOutliner::emitOMPTaskGroupDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.TASKGROUP", "DIR.OMP.END.TASKGROUP",
                             OMPD_taskgroup);
  if (CodeGenFunction::requiresImplicitTaskgroup(Directive)) {
    ClauseEmissionHelper CEH(*this, OMPC_unknown);
    addArg("QUAL.OMP.IMPLICIT");
  }
}
void OpenMPLateOutliner::emitOMPTaskWaitDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.TASKWAIT", "DIR.OMP.END.TASKWAIT",
                             OMPD_taskwait);
}
void OpenMPLateOutliner::emitOMPTaskYieldDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.TASKYIELD", "DIR.OMP.END.TASKYIELD",
                             OMPD_taskyield);
}
void OpenMPLateOutliner::emitOMPBarrierDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.BARRIER", "DIR.OMP.END.BARRIER",
                             OMPD_barrier);
}
void OpenMPLateOutliner::emitOMPFlushDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.FLUSH", "DIR.OMP.END.FLUSH", OMPD_flush);
}
void OpenMPLateOutliner::emitOMPTeamsDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.TEAMS", "DIR.OMP.END.TEAMS", OMPD_teams);
}
void OpenMPLateOutliner::emitOMPDistributeDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.DISTRIBUTE", "DIR.OMP.END.DISTRIBUTE",
                             OMPD_distribute);
}
void OpenMPLateOutliner::emitOMPDistributeParallelForDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.DISTRIBUTE.PARLOOP",
                             "DIR.OMP.END.DISTRIBUTE.PARLOOP",
                             OMPD_distribute_parallel_for);
}
void OpenMPLateOutliner::emitOMPDistributeParallelForSimdDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.DISTRIBUTE.PARLOOP",
                             "DIR.OMP.END.DISTRIBUTE.PARLOOP",
                             OMPD_distribute_parallel_for);
  startDirectiveIntrinsicSet("DIR.OMP.SIMD", "DIR.OMP.END.SIMD", OMPD_simd);
}
void OpenMPLateOutliner::emitOMPDistributeSimdDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.DISTRIBUTE", "DIR.OMP.END.DISTRIBUTE",
                             OMPD_distribute);
  startDirectiveIntrinsicSet("DIR.OMP.SIMD", "DIR.OMP.END.SIMD", OMPD_simd);
}
void OpenMPLateOutliner::emitOMPSectionsDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.SECTIONS", "DIR.OMP.END.SECTIONS",
                             OMPD_sections);
}
void OpenMPLateOutliner::emitOMPSectionDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.SECTION", "DIR.OMP.END.SECTION",
                             OMPD_section);
}
void OpenMPLateOutliner::emitOMPParallelSectionsDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.PARALLEL.SECTIONS",
                             "DIR.OMP.END.PARALLEL.SECTIONS",
                             OMPD_parallel_sections);
}

static StringRef getCancelQualString(OpenMPDirectiveKind Kind) {
  switch (Kind) {
  case OMPD_parallel:
    return "PARALLEL";
  case OMPD_sections:
    return "SECTIONS";
  case OMPD_for:
    return "LOOP";
  case OMPD_taskgroup:
    return "TASKGROUP";
  default:
    llvm_unreachable("Unexpected cancel region type");
  }
}

void OpenMPLateOutliner::emitOMPCancelDirective(OpenMPDirectiveKind Kind) {
  startDirectiveIntrinsicSet("DIR.OMP.CANCEL", "DIR.OMP.END.CANCEL",
                             OMPD_cancel);
  ClauseEmissionHelper CEH(*this, OMPC_unknown, "QUAL.OMP.CANCEL.");
  ClauseStringBuilder &CSB = CEH.getBuilder();
  CSB.add(getCancelQualString(Kind));
  addArg(CSB.getString());
}

void OpenMPLateOutliner::emitOMPCancellationPointDirective(
    OpenMPDirectiveKind Kind) {
  startDirectiveIntrinsicSet("DIR.OMP.CANCELLATION.POINT",
                             "DIR.OMP.END.CANCELLATION.POINT",
                             OMPD_cancellation_point);
  ClauseEmissionHelper CEH(*this, OMPC_unknown, "QUAL.OMP.CANCEL.");
  ClauseStringBuilder &CSB = CEH.getBuilder();
  CSB.add(getCancelQualString(Kind));
  addArg(CSB.getString());
}

/// return true if clause is not allowed in current pragma.
bool OpenMPLateOutliner::shouldSkipExplicitClause(OpenMPClauseKind Kind) {
  if (isImplicitTask(OMPD_target) || isImplicitTask(OMPD_dispatch)) {
    return Kind == OMPC_depend || Kind == OMPC_in_reduction ||
           !isAllowedClauseForDirective(CurrentDirectiveKind, Kind,
                                        CGF.getLangOpts().OpenMP);
  }
  if (isImplicitTask(OMPD_task)) {
    return Kind != OMPC_map &&
           !isAllowedClauseForDirective(CurrentDirectiveKind, Kind,
                                        CGF.getLangOpts().OpenMP);
  }
  if (isImplicitTaskgroup(OMPD_taskgroup)) {
    return Kind != OMPC_reduction &&
           !isAllowedClauseForDirective(
               CurrentDirectiveKind,
               Kind == OMPC_reduction ? OMPC_task_reduction : Kind,
               CGF.getLangOpts().OpenMP);
  }
  return !isAllowedClauseForDirective(CurrentDirectiveKind, Kind,
                                      CGF.getLangOpts().OpenMP);
}

void OpenMPLateOutliner::emitOMPTargetVariantDispatchDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.TARGET.VARIANT.DISPATCH",
                             "DIR.OMP.END.TARGET.VARIANT.DISPATCH",
                             OMPD_target_variant_dispatch);
}

void OpenMPLateOutliner::emitOMPDispatchDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.DISPATCH", "DIR.OMP.END.DISPATCH",
                             OMPD_dispatch);
}

void OpenMPLateOutliner::emitOMPGenericLoopDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.GENERICLOOP", "DIR.OMP.END.GENERICLOOP",
                             OMPD_loop);
}

void OpenMPLateOutliner::emitOMPInteropDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.INTEROP", "DIR.OMP.END.INTEROP",
                             OMPD_interop);
}

void OpenMPLateOutliner::emitOMPPrefetchDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.PREFETCH", "DIR.OMP.END.PREFETCH",
                             OMPD_prefetch);
}

void OpenMPLateOutliner::emitOMPScopeDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.SCOPE", "DIR.OMP.END.SCOPE",
                             OMPD_scope);
}

OpenMPLateOutliner &OpenMPLateOutliner::
operator<<(ArrayRef<OMPClause *> Clauses) {
  for (auto *C : Clauses) {
    CurrentClause = C;
    OpenMPClauseKind ClauseKind = C->getClauseKind();
    if (shouldSkipExplicitClause(ClauseKind))
      continue;
    if (ClauseKind == OMPC_map || ClauseKind == OMPC_to ||
        ClauseKind == OMPC_from)
      continue;
    switch (ClauseKind) {
#define GEN_CLANG_CLAUSE_CLASS
#define CLAUSE_CLASS(Enum, Str, Class)                                         \
  case llvm::omp::Clause::Enum:                                                \
    emit##Class(cast<Class>(C));                                               \
    break;
#define CLAUSE_NO_CLASS(Enum, Str)                                             \
  case llvm::omp::Clause::Enum:                                                \
    llvm_unreachable("Clause not allowed");
#include "llvm/Frontend/OpenMP/OMP.inc"
    }
  }
  if (!shouldSkipExplicitClause(OMPC_depend))
    emitOMPAllDependClauses();
  if (!shouldSkipExplicitClause(OMPC_map) ||
      !shouldSkipExplicitClause(OMPC_from) ||
      !shouldSkipExplicitClause(OMPC_to))
    emitOMPAllMapClauses();
  CurrentClause = nullptr;
  return *this;
}

bool OpenMPLateOutliner::isFirstDirectiveInSet(const OMPExecutableDirective &S,
                                               OpenMPDirectiveKind Kind) {
  if (Kind == OMPD_unknown)
    return true;

  if (CodeGenFunction::requiresImplicitTask(S))
    return Kind == OMPD_task;

  OpenMPDirectiveKind DKind = S.getDirectiveKind();
  if (CodeGenFunction::requiresImplicitTaskgroup(S, /*TopLevel=*/true))
    return Kind == OMPD_taskgroup;
  if (Kind == DKind)
    return true;

  switch (DKind) {
  case OMPD_target_parallel:
  case OMPD_target_parallel_for:
  case OMPD_target_parallel_for_simd:
  case OMPD_target_simd:
  case OMPD_target_teams:
  case OMPD_target_teams_distribute:
  case OMPD_target_teams_distribute_simd:
  case OMPD_target_teams_distribute_parallel_for:
  case OMPD_target_teams_distribute_parallel_for_simd:
  case OMPD_target_teams_loop:
  case OMPD_target_parallel_loop:
  case OMPD_target:
  case OMPD_target_enter_data:
  case OMPD_target_exit_data:
  case OMPD_target_update:
    return Kind == OMPD_target;

  case OMPD_teams_distribute:
  case OMPD_teams_distribute_simd:
  case OMPD_teams_distribute_parallel_for:
  case OMPD_teams_distribute_parallel_for_simd:
  case OMPD_teams_loop:
    return Kind == OMPD_teams;

  case OMPD_master_taskloop:
  case OMPD_master_taskloop_simd:
    return Kind == OMPD_master;

  case OMPD_masked_taskloop:
  case OMPD_masked_taskloop_simd:
    return Kind == OMPD_masked;
  
  case OMPD_parallel_masked:
  case OMPD_parallel_master:
  case OMPD_parallel_master_taskloop:
  case OMPD_parallel_master_taskloop_simd:
  case OMPD_parallel_masked_taskloop:
  case OMPD_parallel_masked_taskloop_simd:
  case OMPD_parallel_loop:
    return Kind == OMPD_parallel;

  default:
    llvm_unreachable("Base directive kind in isFirstDirectiveInSet");
  }
}

bool OpenMPLateOutliner::needsVLAExprEmission() {
  // This is required, at least, when outlining the region. I tried to
  // choose a conservative set that return false.
  switch (CurrentDirectiveKind) {
  case OMPD_target:
  case OMPD_target_teams:
  case OMPD_target_parallel:
  case OMPD_target_simd:
  case OMPD_target_parallel_for:
  case OMPD_target_parallel_for_simd:
  case OMPD_target_teams_distribute:
  case OMPD_target_teams_distribute_simd:
  case OMPD_target_teams_distribute_parallel_for:
  case OMPD_target_teams_distribute_parallel_for_simd:
  case OMPD_target_teams_loop:
  case OMPD_target_parallel_loop:
  case OMPD_parallel:
  case OMPD_for:
  case OMPD_parallel_for:
  case OMPD_parallel_sections:
  case OMPD_for_simd:
  case OMPD_parallel_for_simd:
  case OMPD_loop:
  case OMPD_task:
  case OMPD_simd:
  case OMPD_sections:
  case OMPD_taskgroup:
  case OMPD_teams:
  case OMPD_target_data:
  case OMPD_target_exit_data:
  case OMPD_target_enter_data:
  case OMPD_distribute:
  case OMPD_distribute_simd:
  case OMPD_distribute_parallel_for:
  case OMPD_distribute_parallel_for_simd:
  case OMPD_teams_distribute:
  case OMPD_teams_distribute_simd:
  case OMPD_teams_distribute_parallel_for:
  case OMPD_teams_distribute_parallel_for_simd:
  case OMPD_teams_loop:
  case OMPD_target_update:
  case OMPD_taskloop:
  case OMPD_taskloop_simd:
  case OMPD_master_taskloop:
  case OMPD_master_taskloop_simd:
  case OMPD_masked_taskloop:
  case OMPD_masked_taskloop_simd:
  case OMPD_parallel_masked:
  case OMPD_parallel_master:
  case OMPD_parallel_master_taskloop:
  case OMPD_parallel_master_taskloop_simd:
  case OMPD_parallel_masked_taskloop:
  case OMPD_parallel_masked_taskloop_simd:
  case OMPD_parallel_loop:
    return true;
  case OMPD_cancel:
  case OMPD_cancellation_point:
  case OMPD_ordered:
  case OMPD_threadprivate:
  case OMPD_allocate:
  case OMPD_section:
  case OMPD_single:
  case OMPD_master:
  case OMPD_masked:
  case OMPD_critical:
  case OMPD_taskyield:
  case OMPD_barrier:
  case OMPD_taskwait:
  case OMPD_atomic:
  case OMPD_flush:
  case OMPD_declare_simd:
  case OMPD_declare_target:
  case OMPD_end_declare_target:
  case OMPD_declare_variant:
  case OMPD_begin_declare_variant:
  case OMPD_end_declare_variant:
  case OMPD_target_variant_dispatch:
  case OMPD_dispatch:
  case OMPD_declare_reduction:
  case OMPD_declare_mapper:
  case OMPD_requires:
  case OMPD_depobj:
  case OMPD_scan:
  case OMPD_prefetch:
  case OMPD_scope:
    return false;
  case OMPD_unknown:
  default:
  llvm_unreachable("Unexpected directive.");
  }
  return true;
}

/// Emit the captured statement body.
static void EmitBody(CodeGenFunction &CGF, const OMPExecutableDirective &D) {
  if (!CGF.HaveInsertPoint())
    return;
  CGF.EHStack.pushTerminate();

  const Stmt *S;
  if (OpenMPLateOutliner::hasCapturedStmt(D))
    S = (cast<CapturedStmt>(D.getInnermostCapturedStmt()))->getCapturedStmt();
  else
    S = D.getAssociatedStmt();

  // The OMP structured-block (captured statement) is a declaration. Create a
  // new exception scope so we can handle any lingering cleanup exceptions
  // (due to ill-formed block) before popping the terminate exception.
  if (isa<DeclStmt>(S)) {
    CodeGenFunction::RunCleanupsScope Scope(CGF);
    CGF.EmitStmt(S);
  } else {
    CGF.EmitStmt(S);
  }

  CGF.EHStack.popTerminate();
}

/// Return true if processing the part of an implicit task corresponding to K.
bool OpenMPLateOutliner::isImplicitTask(OpenMPDirectiveKind K) {
  if (!CodeGenFunction::requiresImplicitTask(Directive))
     return false;
   return K == CurrentDirectiveKind;
}

/// Return true if processing the part of an implicit taskgroup corresponding
/// to K.
bool OpenMPLateOutliner::isImplicitTaskgroup(OpenMPDirectiveKind K) {
  if (!CodeGenFunction::requiresImplicitTaskgroup(Directive))
     return false;
   return K == CurrentDirectiveKind;
}

/// Return true if Directive require implicit taskgroup
bool CodeGenFunction::requiresImplicitTaskgroup(
    const OMPExecutableDirective &Directive, bool TopLevel) {
  OpenMPDirectiveKind DKind = Directive.getDirectiveKind();
  if (TopLevel && DKind != OMPD_taskloop && DKind != OMPD_taskloop_simd)
     return false;
  if (isOpenMPTaskLoopDirective(DKind) &&
      !Directive.hasClausesOfKind<OMPNogroupClause>())
    return true;
  return false;
}

/// Return true if Directive requires implicit task to be generated.
bool CodeGenFunction::requiresImplicitTask(
    const OMPExecutableDirective &Directive) {
  bool HasDependOrNowait =
      Directive.hasClausesOfKind<OMPDependClause>() ||
      Directive.hasClausesOfKind<OMPNowaitClause>();
  if (isOpenMPTargetExecutionDirective(Directive.getDirectiveKind()) &&
      (HasDependOrNowait || Directive.hasClausesOfKind<OMPInReductionClause>()))
    return true;
  if ((Directive.getDirectiveKind() == OMPD_target_enter_data ||
       Directive.getDirectiveKind() == OMPD_target_exit_data ||
       Directive.getDirectiveKind() == OMPD_dispatch ||
       Directive.getDirectiveKind() == OMPD_target_update) &&
       HasDependOrNowait)
    return true;
  return false;
}

/// Retrieve the value of the context parameter.
llvm::Value *CGLateOutlineOpenMPRegionInfo::getContextValue() const {
  if (OldCSI)
    return OldCSI->getContextValue();
  llvm_unreachable("No context value for inlined OpenMP region");
}
void CGLateOutlineOpenMPRegionInfo::setContextValue(llvm::Value *V) {
  if (OldCSI) {
    OldCSI->setContextValue(V);
    return;
  }
  llvm_unreachable("No context value for inlined OpenMP region");
}
/// Lookup the captured field decl for a variable.
const FieldDecl *CGLateOutlineOpenMPRegionInfo::lookup(const VarDecl *VD) const {
  if (OldCSI)
    return OldCSI->lookup(VD);
  // If there is no outer outlined region,no need to lookup in a list of
  // captured variables, we can use the original one.
  return nullptr;
}
FieldDecl *CGLateOutlineOpenMPRegionInfo::getThisFieldDecl() const {
  if (OldCSI)
    return OldCSI->getThisFieldDecl();
  return nullptr;
}

bool CGLateOutlineOpenMPRegionInfo::isDispatchTargetCall(SourceLocation Loc) {
  if (!inDispatchRegion())
    return false;
  const auto *DispatchD = cast<OMPDispatchDirective>(&D);
  const SourceLocation TLoc = DispatchD->getTargetCallLoc();
  return Loc.getRawEncoding() == TLoc.getRawEncoding();
}
static OpenMPDirectiveKind
nextDirectiveKind(const OMPExecutableDirective &Directive,
                  OpenMPDirectiveKind CurrDirKind) {

  OpenMPDirectiveKind FullDirKind = Directive.getDirectiveKind();
  if (CurrDirKind == OMPD_task) {
    if (FullDirKind == OMPD_target_enter_data ||
        FullDirKind == OMPD_target_exit_data ||
        FullDirKind == OMPD_dispatch ||
        FullDirKind == OMPD_target_update)
      return FullDirKind;
    return OMPD_target;
  }

  if (CurrDirKind == OMPD_taskgroup &&
      (FullDirKind == OMPD_taskloop || FullDirKind == OMPD_taskloop_simd))
    return FullDirKind;

  switch (FullDirKind) {
  case OMPD_target_parallel:
    // OMPD_target -> OMPD_parallel
    if (CurrDirKind == OMPD_target)
      return OMPD_parallel;
    return OMPD_unknown;

  case OMPD_target_parallel_for:
    // OMPD_target -> OMPD_parallel_for
    if (CurrDirKind == OMPD_target)
      return OMPD_parallel_for;
    return OMPD_unknown;

  case OMPD_target_parallel_for_simd:
    // OMPD_target -> OMPD_parallel_for_simd
    if (CurrDirKind == OMPD_target)
      return OMPD_parallel_for_simd;
    return OMPD_unknown;

  case OMPD_target_simd:
    // OMPD_target -> OMPD_simd
    if (CurrDirKind == OMPD_target)
      return OMPD_simd;
    return OMPD_unknown;

  case OMPD_target_teams:
    // OMPD_target -> OMPD_teams
    if (CurrDirKind == OMPD_target)
      return OMPD_teams;
    return OMPD_unknown;

  case OMPD_target_teams_distribute:
  case OMPD_teams_distribute:
    // OMPD_target -> OMPD_teams -> OMPD_distribute
    if (CurrDirKind == OMPD_target)
      return OMPD_teams;
    if (CurrDirKind == OMPD_teams)
      return OMPD_distribute;
    return OMPD_unknown;

  case OMPD_teams_loop:
    // OMPD_teams -> omp_loop
    if (CurrDirKind == OMPD_teams)
      return OMPD_loop;
    return OMPD_unknown;

  case OMPD_target_teams_loop:
    //  OMPD_target -> OMPD_teams -> OMPD_loop
    if (CurrDirKind == OMPD_target)
      return OMPD_teams;
    if (CurrDirKind == OMPD_teams)
      return OMPD_loop;
    return OMPD_unknown;

  case OMPD_target_parallel_loop:
    //  OMPD_target -> OMPD_parallel -> OMPD_loop
    if (CurrDirKind == OMPD_target)
      return OMPD_parallel;
    if (CurrDirKind == OMPD_parallel)
      return OMPD_loop;
    return OMPD_unknown;

  case OMPD_parallel_loop:
    // OMPD_parallel -> omp_loop
    if (CurrDirKind == OMPD_parallel)
      return OMPD_loop;
    return OMPD_unknown;

  case OMPD_target_teams_distribute_simd:
  case OMPD_teams_distribute_simd:
    // OMPD_target -> OMPD_teams -> OMPD_distribute_simd
    if (CurrDirKind == OMPD_target)
      return OMPD_teams;
    if (CurrDirKind == OMPD_teams)
      return OMPD_distribute_simd;
    return OMPD_unknown;

  case OMPD_target_teams_distribute_parallel_for:
  case OMPD_teams_distribute_parallel_for:
    // OMPD_target -> OMPD_teams -> OMPD_distribute_parallel_for
    if (CurrDirKind == OMPD_target)
      return OMPD_teams;
    if (CurrDirKind == OMPD_teams)
      return OMPD_distribute_parallel_for;
    return OMPD_unknown;

  case OMPD_target_teams_distribute_parallel_for_simd:
  case OMPD_teams_distribute_parallel_for_simd:
    // OMPD_target -> OMPD_teams -> OMPD_distribute_parallel_for_simd
    if (CurrDirKind == OMPD_target)
      return OMPD_teams;
    if (CurrDirKind == OMPD_teams)
      return OMPD_distribute_parallel_for_simd;
    return OMPD_unknown;

  case OMPD_target:
  case OMPD_target_enter_data:
  case OMPD_target_exit_data:
  case OMPD_target_update:
    return OMPD_unknown;

  case OMPD_master_taskloop:
    // OMPD_master -> OMPD_taskgroup -> OMPD_taskloop
    // OMPD_master -> OMPD_taskloop
    if (CurrDirKind == OMPD_master)
      return CodeGenFunction::requiresImplicitTaskgroup(Directive)
                 ? OMPD_taskgroup
                 : OMPD_taskloop;
    if (CurrDirKind == OMPD_taskgroup)
      return OMPD_taskloop;
    return OMPD_unknown;

  case OMPD_master_taskloop_simd:
    // OMPD_master -> OMPD_taskgroup -> OMPD_taskloop -> OMPD_simd
    // OMPD_master -> OMPD_taskloop_simd
    if (CurrDirKind == OMPD_master)
      return CodeGenFunction::requiresImplicitTaskgroup(Directive)
                 ? OMPD_taskgroup
                 : OMPD_taskloop_simd;
    if (CurrDirKind == OMPD_taskgroup)
      return OMPD_taskloop_simd;
    return OMPD_unknown;

  case OMPD_masked_taskloop:
    // OMPD_masked -> OMPD_taskgroup -> OMPD_taskloop
    // OMPD_masked -> OMPD_taskloop
    if (CurrDirKind == OMPD_masked)
      return CodeGenFunction::requiresImplicitTaskgroup(Directive)
                 ? OMPD_taskgroup
                 : OMPD_taskloop;
    if (CurrDirKind == OMPD_taskgroup)
      return OMPD_taskloop;
    return OMPD_unknown;
  
  case OMPD_masked_taskloop_simd:
    // OMPD_masked -> OMPD_taskgroup -> OMPD_taskloop -> OMPD_simd
    // OMPD_masked -> OMPD_taskloop_simd
    if (CurrDirKind == OMPD_masked)
      return CodeGenFunction::requiresImplicitTaskgroup(Directive)
                 ? OMPD_taskgroup
                 : OMPD_taskloop_simd;
    if (CurrDirKind == OMPD_taskgroup)
      return OMPD_taskloop_simd;
    return OMPD_unknown;

  case OMPD_parallel_masked:
    // OMPD_parallel -> OMPD_masked
    if (CurrDirKind == OMPD_parallel)
      return OMPD_masked;
    return OMPD_unknown;
  
  case OMPD_parallel_master:
    // OMPD_parallel -> OMPD_master
    if (CurrDirKind == OMPD_parallel)
      return OMPD_master;
    return OMPD_unknown;
  
  case OMPD_parallel_master_taskloop:
    // OMPD_parallel -> OMPD_master -> OMPD_taskgroup ->OMPD_taskloop
    // OMPD_parallel -> OMPD_master -> OMPD_taskloop
    if (CurrDirKind == OMPD_parallel)
      return OMPD_master;
    if (CurrDirKind == OMPD_master)
      return CodeGenFunction::requiresImplicitTaskgroup(Directive)
                 ? OMPD_taskgroup
                 : OMPD_taskloop;
    if (CurrDirKind == OMPD_taskgroup)
      return OMPD_taskloop;
    return OMPD_unknown;

  case OMPD_parallel_master_taskloop_simd:
    // OMPD_parallel -> OMPD_master -> OMPD_taskgroup -> OMPD_taskloop ->
    // OMPD_simd
    // OMPD_parallel -> OMPD_master -> OMPD_taskloop_simd
    if (CurrDirKind == OMPD_parallel)
      return OMPD_master;
    if (CurrDirKind == OMPD_master)
      return CodeGenFunction::requiresImplicitTaskgroup(Directive)
                 ? OMPD_taskgroup
                 : OMPD_taskloop_simd;
    if (CurrDirKind == OMPD_taskgroup)
      return OMPD_taskloop_simd;
    return OMPD_unknown;

  case OMPD_parallel_masked_taskloop:
    // OMPD_parallel -> OMPD_masked -> OMPD_taskgroup ->OMPD_taskloop
    // OMPD_parallel -> OMPD_masked -> OMPD_taskloop
    if (CurrDirKind == OMPD_parallel)
      return OMPD_masked;
    if (CurrDirKind == OMPD_masked)
      return CodeGenFunction::requiresImplicitTaskgroup(Directive)
                 ? OMPD_taskgroup
                 : OMPD_taskloop;
    if (CurrDirKind == OMPD_taskgroup)
      return OMPD_taskloop;
    return OMPD_unknown;

  case OMPD_parallel_masked_taskloop_simd:
    // OMPD_parallel -> OMPD_masked -> OMPD_taskgroup -> OMPD_taskloop ->
    // OMPD_simd
    // OMPD_parallel -> OMPD_masked -> OMPD_taskloop_simd
    if (CurrDirKind == OMPD_parallel)
      return OMPD_masked;
    if (CurrDirKind == OMPD_masked)
      return CodeGenFunction::requiresImplicitTaskgroup(Directive)
                 ? OMPD_taskgroup
                 : OMPD_taskloop_simd;
    if (CurrDirKind == OMPD_taskgroup)
      return OMPD_taskloop_simd;
    return OMPD_unknown;

  default:
    llvm_unreachable("Unhandled combined directive.");
  }
}

static void addAttrsForFuncWithTargetRegion(llvm::Function *F) {
    F->addFnAttr("contains-openmp-target", "true");
    if (F->hasFnAttribute(llvm::Attribute::AlwaysInline))
      F->removeFnAttr(llvm::Attribute::AlwaysInline);
    F->addFnAttr(llvm::Attribute::NoInline);
}

void CodeGenFunction::EmitLateOutlineOMPDirective(
    const OMPExecutableDirective &S, OpenMPDirectiveKind Kind) {
  TerminateHandlerRAII THandler(*this);
  OMPLateOutlineLexicalScope Scope(*this, S, Kind);
  OpenMPLateOutliner Outliner(*this, S, Kind);
  OpenMPDirectiveKind CurrentDirectiveKind = Outliner.getCurrentDirectiveKind();
#if INTEL_CUSTOMIZATION
  HoistLoopBoundsIfPossible(S, CurrentDirectiveKind);
#endif // INTEL_CUSTOMIZATION
  HoistTeamsClausesIfPossible(S, CurrentDirectiveKind);

  switch (CurrentDirectiveKind) {
  case OMPD_parallel:
    Outliner.emitOMPParallelDirective();
    break;
  case OMPD_atomic: {
    bool IsSeqCst = S.hasClausesOfKind<OMPSeqCstClause>();
    OMPAtomicClause ClauseKind = IsSeqCst ? OMP_update_seq_cst : OMP_update;
    if (S.hasClausesOfKind<OMPReadClause>())
      ClauseKind = IsSeqCst ? OMP_read_seq_cst : OMP_read;
    else if (S.hasClausesOfKind<OMPWriteClause>())
      ClauseKind = IsSeqCst ? OMP_write_seq_cst : OMP_write;
    else if (S.hasClausesOfKind<OMPCaptureClause>())
      ClauseKind = IsSeqCst ? OMP_capture_seq_cst : OMP_capture;
    Outliner.emitOMPAtomicDirective(ClauseKind);
    break;
  }
  case OMPD_single:
    Outliner.emitOMPSingleDirective();
    break;
  case OMPD_master:
    Outliner.emitOMPMasterDirective();
    break;
  case OMPD_masked:
    Outliner.emitOMPMaskedDirective();
    break;
  case OMPD_critical: {
    const auto &CD = cast<OMPCriticalDirective>(S);
    Outliner.emitOMPCriticalDirective(CD.getDirectiveName().getAsString());
    break;
  }
  case OMPD_ordered:
    Outliner.emitOMPOrderedDirective();
    break;
  case OMPD_scan:
    Outliner.emitOMPScanDirective();
    break;
  case OMPD_target: {
#if INTEL_CUSTOMIZATION
    CGM.setHasTargetCode();
#endif //INTEL_CUSTOMIZATION

    // Get an index of the associated offload entry for this target directive.
    assert(CurFuncDecl && "No parent declaration for target region!");
    StringRef ParentName;
    std::string ItaniumMangledName;
    // In case we have Ctors/Dtors we use the complete type variant to produce
    // the mangling of the device outlined kernel.
    if (const auto *D = dyn_cast<CXXConstructorDecl>(CurFuncDecl)) {
      ParentName = CGM.getMangledName(GlobalDecl(D, Ctor_Complete));
#if INTEL_CUSTOMIZATION
      if (CGM.getLangOpts().OpenMPLateOutlineTarget)
#endif // INTEL_CUSTOMIZATION
      if (CGM.getLangOpts().OpenMPLateOutline) {
        ItaniumMangledName = CGM.getUniqueItaniumABIMangledName(
            GlobalDecl(D, Ctor_Complete));
        ParentName = ItaniumMangledName;
      }
    } else if (const auto *D = dyn_cast<CXXDestructorDecl>(CurFuncDecl)) {
      ParentName = CGM.getMangledName(GlobalDecl(D, Dtor_Complete));
#if INTEL_CUSTOMIZATION
      if (CGM.getLangOpts().OpenMPLateOutlineTarget)
#endif // INTEL_CUSTOMIZATION
      if (CGM.getLangOpts().OpenMPLateOutline) {
        ItaniumMangledName = CGM.getUniqueItaniumABIMangledName(
            GlobalDecl(D, Dtor_Complete));
        ParentName = ItaniumMangledName;
      }
    } else {
      ParentName =
          CGM.getMangledName(GlobalDecl(cast<FunctionDecl>(CurFuncDecl)));
#if INTEL_CUSTOMIZATION
      if (CGM.getLangOpts().OpenMPLateOutlineTarget)
#endif // INTEL_CUSTOMIZATION
      if (CGM.getLangOpts().OpenMPLateOutline) {
        ItaniumMangledName = CGM.getUniqueItaniumABIMangledName(
            GlobalDecl(cast<FunctionDecl>(CurFuncDecl)));
        ParentName = ItaniumMangledName;
      }
    }

    int Order = CGM.getOpenMPRuntime().registerTargetRegion(S, ParentName);
    if (Order >= 0)
      Outliner.emitOMPTargetDirective(Order);
    break;
  }
  case OMPD_target_data:
#if INTEL_CUSTOMIZATION
    CGM.setHasTargetCode();
#endif // INTEL_CUSTOMIZATION
    Outliner.emitOMPTargetDataDirective();
    break;
  case OMPD_target_update:
#if INTEL_CUSTOMIZATION
    CGM.setHasTargetCode();
#endif // INTEL_CUSTOMIZATION
    Outliner.emitOMPTargetUpdateDirective();
    break;
  case OMPD_target_enter_data:
#if INTEL_CUSTOMIZATION
    CGM.setHasTargetCode();
#endif // INTEL_CUSTOMIZATION
    Outliner.emitOMPTargetEnterDataDirective();
    break;
  case OMPD_target_exit_data:
#if INTEL_CUSTOMIZATION
    CGM.setHasTargetCode();
#endif // INTEL_CUSTOMIZATION
    Outliner.emitOMPTargetExitDataDirective();
    break;
  case OMPD_task:
    Outliner.emitOMPTaskDirective();
    break;
  case OMPD_taskgroup:
    Outliner.emitOMPTaskGroupDirective();
    break;
  case OMPD_taskwait:
    Outliner.emitOMPTaskWaitDirective();
    break;
  case OMPD_taskyield:
    Outliner.emitOMPTaskYieldDirective();
    break;
  case OMPD_teams:
    Outliner.emitOMPTeamsDirective();
    break;
  case OMPD_barrier:
    Outliner.emitOMPBarrierDirective();
    break;
  case OMPD_flush:
    Outliner.emitOMPFlushDirective();
    break;
  case OMPD_sections:
    Outliner.emitOMPSectionsDirective();
    break;
  case OMPD_section:
    Outliner.emitOMPSectionDirective();
    break;
  case OMPD_parallel_sections:
    Outliner.emitOMPParallelSectionsDirective();
    break;
  case OMPD_cancel:
    Outliner.emitOMPCancelDirective(
        cast<OMPCancelDirective>(S).getCancelRegion());
    break;
  case OMPD_cancellation_point:
    Outliner.emitOMPCancellationPointDirective(
        cast<OMPCancellationPointDirective>(S).getCancelRegion());
    break;
  case OMPD_target_variant_dispatch:
    Outliner.emitOMPTargetVariantDispatchDirective();
    break;
  case OMPD_dispatch:
    Outliner.emitOMPDispatchDirective();
    break;

  case OMPD_interop:
    Outliner.emitOMPInteropDirective();
    break;

  case OMPD_prefetch:
    Outliner.emitOMPPrefetchDirective();
    break;

  case OMPD_scope:
    Outliner.emitOMPScopeDirective();
    break;

  // These directives are not yet implemented.
  case OMPD_requires:
  case OMPD_depobj:
  case OMPD_tile:
  case OMPD_unroll:
    break;

  // These directives do not create region directives.
  case OMPD_allocate:
  case OMPD_declare_target:
  case OMPD_end_declare_target:
  case OMPD_declare_variant:
  case OMPD_begin_declare_variant:
  case OMPD_end_declare_variant:
  case OMPD_threadprivate:
  case OMPD_declare_reduction:
  case OMPD_declare_simd:
  case OMPD_declare_mapper:
  case OMPD_unknown:
    llvm_unreachable("Wrong OpenMP directive");

  case OMPD_distribute:
  case OMPD_distribute_simd:
  case OMPD_distribute_parallel_for:
  case OMPD_distribute_parallel_for_simd:
  case OMPD_simd:
  case OMPD_for:
  case OMPD_for_simd:
  case OMPD_parallel_for:
  case OMPD_parallel_for_simd:
  case OMPD_taskloop:
  case OMPD_taskloop_simd:
  case OMPD_loop:
    llvm_unreachable("OpenMP loops not handled here");

  case OMPD_target_parallel:
  case OMPD_target_parallel_for:
  case OMPD_target_parallel_for_simd:
  case OMPD_target_simd:
  case OMPD_target_teams:
  case OMPD_target_teams_distribute:
  case OMPD_target_teams_distribute_simd:
  case OMPD_target_teams_distribute_parallel_for:
  case OMPD_target_teams_distribute_parallel_for_simd:
  case OMPD_target_teams_loop:
  case OMPD_target_parallel_loop:
  case OMPD_teams_distribute:
  case OMPD_teams_distribute_simd:
  case OMPD_teams_distribute_parallel_for:
  case OMPD_teams_distribute_parallel_for_simd:
  case OMPD_teams_loop:
  case OMPD_master_taskloop:
  case OMPD_master_taskloop_simd:
  case OMPD_masked_taskloop:
  case OMPD_masked_taskloop_simd:
  case OMPD_parallel_master:
  case OMPD_parallel_master_taskloop:
  case OMPD_parallel_master_taskloop_simd:
  case OMPD_parallel_masked_taskloop:
  case OMPD_parallel_masked_taskloop_simd:
  case OMPD_parallel_loop:
  default:
    llvm_unreachable("Combined directives not handled here");
  }
  Outliner << S.clauses();
  Outliner.insertMarker();
  if (S.hasAssociatedStmt() && S.getAssociatedStmt() != nullptr) {
    LateOutlineOpenMPRegionRAII Region(*this, Outliner, S);
    CodeGenFunction::OMPPrivateScope MapClausePointerScope(*this);
    Outliner.privatizeMappedPointers(MapClausePointerScope);
    if (S.getDirectiveKind() != CurrentDirectiveKind) {
      // Unless we've reached the innermost directive, keep going.
      OpenMPDirectiveKind NextKind = nextDirectiveKind(S, CurrentDirectiveKind);
      switch (NextKind) {
        case OMPD_parallel:
        case OMPD_teams:
        case OMPD_target:
        case OMPD_master:
        case OMPD_masked:
        case OMPD_target_enter_data:
        case OMPD_target_exit_data:
        case OMPD_target_update:
        case OMPD_dispatch:
        case OMPD_taskgroup:
          return EmitLateOutlineOMPDirective(S, NextKind);
        case OMPD_parallel_for:
        case OMPD_parallel_for_simd:
        case OMPD_simd:
        case OMPD_distribute:
        case OMPD_distribute_simd:
        case OMPD_distribute_parallel_for:
        case OMPD_distribute_parallel_for_simd:
        case OMPD_taskloop:
        case OMPD_taskloop_simd:
        case OMPD_loop:
          return EmitLateOutlineOMPLoopDirective(cast<OMPLoopDirective>(S),
                                                 NextKind);
        case OMPD_unknown:
          // This is the innermost directive, fallthrough.
          break;
        default:
          llvm_unreachable("Unexpected next directive kind.");
      }
    }
    bool IsDeviceTarget = getLangOpts().OpenMPIsDevice &&
      isOpenMPTargetExecutionDirective(S.getDirectiveKind());
    if (IsDeviceTarget)
      addAttrsForFuncWithTargetRegion(CurFn);
    CodeGenModule::InTargetRegionRAII ITR(CGM, IsDeviceTarget);
    Outliner.emitVLAExpressions();
    EmitBody(*this, S);
  }
}

void CodeGenFunction::RemapForLateOutlining(const OMPExecutableDirective &D,
                                            OMPPrivateScope &PrivScope) {
  SmallVector<const Expr *, 5> RemapVars;
  for (const auto *C : D.getClausesOfKind<OMPPrivateClause>())
     for (const auto *Ref : C->varlists())
       RemapVars.push_back(Ref);
  for (const auto *C : D.getClausesOfKind<OMPFirstprivateClause>())
     for (const auto *Ref : C->varlists())
       RemapVars.push_back(Ref);
  for (const auto *C : D.getClausesOfKind<OMPLastprivateClause>())
     for (const auto *Ref : C->varlists())
       RemapVars.push_back(Ref);
  for (const auto *C : D.getClausesOfKind<OMPUseDeviceAddrClause>())
     for (const auto *Ref : C->varlists())
       RemapVars.push_back(Ref);
  for (const auto *C : D.getClausesOfKind<OMPReductionClause>())
     for (const auto *Ref : C->varlists()) {
       const Expr *VarExpr = OpenMPLateOutliner::getExplicitDeclRefOrNull(Ref);
       RemapVars.push_back(VarExpr ? VarExpr : Ref);
     }
  for (const auto *C : D.getClausesOfKind<OMPMapClause>())
     for (const auto *Ref : C->varlists()) {
       const Expr *VarExpr = OpenMPLateOutliner::getExplicitDeclRefOrNull(Ref);
       RemapVars.push_back(VarExpr ? VarExpr : Ref);
     }
  for (const auto *C : D.getClausesOfKind<OMPUseDevicePtrClause>())
     for (const auto *Ref : C->varlists()) {
       const Expr *VarExpr = OpenMPLateOutliner::getExplicitDeclRefOrNull(Ref);
       RemapVars.push_back(VarExpr ? VarExpr : Ref);
     }
  for (const auto *C : D.getClausesOfKind<OMPIsDevicePtrClause>())
     for (const auto *Ref : C->varlists()) {
       const Expr *VarExpr = OpenMPLateOutliner::getExplicitDeclRefOrNull(Ref);
       RemapVars.push_back(VarExpr ? VarExpr : Ref);
     }
  for (const auto *C : D.getClausesOfKind<OMPHasDeviceAddrClause>())
     for (const auto *Ref : C->varlists()) {
       const Expr *VarExpr = OpenMPLateOutliner::getExplicitDeclRefOrNull(Ref);
       RemapVars.push_back(VarExpr ? VarExpr : Ref);
     }
  for (const auto *Ref : RemapVars) {
    if (auto *DRE = dyn_cast<DeclRefExpr>(Ref->IgnoreParenImpCasts())) {
      if (auto *VD = dyn_cast<VarDecl>(DRE->getDecl())) {
        if (isa<OMPCapturedExprDecl>(VD)) {
          PrivScope.addPrivateNoTemps(VD, [this, VD]() -> Address {
            Address A = EmitLValue(VD->getAnyInitializer()).getAddress(*this);
            if (VD->getType()->isReferenceType())
              A.setRemovedReference();
            return A;
          });
        }
      }
    }
  }
}

static void emitLateOutlineDirective(CodeGenFunction &CGF,
                                     const RegionCodeGenTy &CodeGen) {
  if (!CGF.HaveInsertPoint())
    return;
  CGF.EHStack.pushTerminate();
  CodeGen(CGF);
  CGF.EHStack.popTerminate();
}

void CodeGenFunction::EmitLateOutlineOMPLoopDirective(
    const OMPLoopDirective &S, OpenMPDirectiveKind Kind) {
  OMPLateOutlineLexicalScope Scope(*this, S, Kind);
  auto &&CodeGen = [&S,Kind](CodeGenFunction &CGF, PrePostActionTy &) {
    CGF.EmitLateOutlineOMPLoop(S, Kind);
  };
  bool IsDeviceTarget = getLangOpts().OpenMPIsDevice &&
    isOpenMPTargetExecutionDirective(S.getDirectiveKind());
  if (IsDeviceTarget)
    addAttrsForFuncWithTargetRegion(CurFn);
  CodeGenModule::InTargetRegionRAII ITR(CGM, IsDeviceTarget);
  emitLateOutlineDirective(*this, CodeGen);
}
#endif // INTEL_COLLAB
