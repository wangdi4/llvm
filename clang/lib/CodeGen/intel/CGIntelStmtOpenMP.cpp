//===--- CGIntelStmtOpenMP.cpp - Emit Intel Code from OpenMP Directives ---===//
//
// Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file contains code to emit OpenMP nodes as LLVM code.
//
//===----------------------------------------------------------------------===//

#if INTEL_SPECIFIC_OPENMP
#include "CGIntelStmtOpenMP.h"
using namespace clang;
using namespace CodeGen;
using namespace CGIntelOpenMP;

llvm::Value *
OpenMPCodeOutliner::emitIntelOpenMPDefaultConstructor(const Expr *IPriv) {

  if (!IPriv)
    return llvm::ConstantPointerNull::get(CGF.VoidPtrTy);
  
  auto *Private = cast<VarDecl>(cast<DeclRefExpr>(IPriv)->getDecl());
  QualType Ty = Private->getType();

  CodeGenModule &CGM = CGF.CGM;
  SmallString<256> OutName;
  llvm::raw_svector_ostream Out(OutName);
  CGM.getCXXABI().getMangleContext().mangleTypeName(Ty, Out);
  Out << ".omp.def_constr";

  if (llvm::Value *F = CGM.GetGlobalValue(OutName))
    return F;

  // Generate function that re-emits the declaration's initializer into the
  // threadprivate copy of the variable VD
  auto &Ctx = CGM.getContext();
  QualType PtrTy = Ctx.getPointerType(Ty);
  CodeGenFunction NewCGF(CGM);
  FunctionArgList Args;
  ImplicitParamDecl Dst(CGM.getContext(), /*DC=*/nullptr, SourceLocation(),
                        /*Id=*/nullptr, PtrTy);
  Args.push_back(&Dst);

  auto &FI = CGM.getTypes().arrangeBuiltinFunctionDeclaration(PtrTy, Args);
  auto FTy = CGM.getTypes().GetFunctionType(FI);
  auto *Fn = CGM.CreateGlobalInitOrDestructFunction(FTy, OutName, FI);
  NewCGF.StartFunction(GlobalDecl(), PtrTy, Fn, FI, Args, SourceLocation());
  auto *Init = Private->getInit();
  if (Init && !NewCGF.isTrivialInitializer(Init)) {
    CodeGenFunction::RunCleanupsScope Scope(NewCGF);
    LValue ArgLVal = NewCGF.EmitLoadOfPointerLValue(
        NewCGF.GetAddrOfLocalVar(&Dst), PtrTy->getAs<PointerType>());
    NewCGF.EmitAnyExprToMem(Init, ArgLVal.getAddress(), Ty.getQualifiers(),
                         /*IsInitializer=*/true);
    NewCGF.Builder.CreateStore(ArgLVal.getPointer(), NewCGF.ReturnValue);
  }
  NewCGF.FinishFunction();
  return Fn;
}

llvm::Value *
OpenMPCodeOutliner::emitIntelOpenMPDestructor(QualType Ty) {
  CodeGenModule &CGM = CGF.CGM; 
  SmallString<256> OutName;
  llvm::raw_svector_ostream Out(OutName);
  CGM.getCXXABI().getMangleContext().mangleTypeName(Ty, Out);
  Out << ".omp.destr";

  if (llvm::Value *F = CGM.GetGlobalValue(OutName))
    return F;

  // Generate function that emits destructor call for the threadprivate copy
  // of the variable VD
  auto &Ctx = CGM.getContext();
  QualType PtrTy = Ctx.getPointerType(Ty);
  CodeGenFunction NewCGF(CGM);
  FunctionArgList Args;
  ImplicitParamDecl Dst(CGM.getContext(), /*DC=*/nullptr, SourceLocation(),
                        /*Id=*/nullptr, PtrTy);
  Args.push_back(&Dst);

  auto &FI = CGM.getTypes().arrangeBuiltinFunctionDeclaration(
      CGM.getContext().VoidTy, Args);
  auto FTy = CGM.getTypes().GetFunctionType(FI);
  auto *Fn = CGM.CreateGlobalInitOrDestructFunction(FTy, OutName, FI);
  NewCGF.StartFunction(GlobalDecl(), CGM.getContext().VoidTy, Fn, FI, Args,
                    SourceLocation());
  if (Ty.isDestructedType() != QualType::DK_none) {
    CodeGenFunction::RunCleanupsScope Scope(NewCGF);
    LValue ArgLVal = NewCGF.EmitLoadOfPointerLValue(
                                                 NewCGF.GetAddrOfLocalVar(&Dst),
                                                 PtrTy->getAs<PointerType>());
    NewCGF.emitDestroy(ArgLVal.getAddress(), Ty,
                    NewCGF.getDestroyer(Ty.isDestructedType()),
                    NewCGF.needsEHCleanup(Ty.isDestructedType()));
  }
  NewCGF.FinishFunction();
  return Fn;
}

llvm::Value *
OpenMPCodeOutliner::emitIntelOpenMPCopyConstructor(const Expr *IPriv) {
  if (!IPriv)
    return llvm::ConstantPointerNull::get(CGF.VoidPtrTy);

  auto *Private = cast<VarDecl>(cast<DeclRefExpr>(IPriv)->getDecl());
  
  CodeGenModule &CGM = CGF.CGM;
  auto &C = CGM.getContext();
  QualType Ty = Private->getType();
  QualType ElemType = Ty;
  if (Ty->isArrayType())
    ElemType = C.getBaseElementType(Ty).getNonReferenceType();

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
  // the code in emitIntelOpenMPDefaultConstructor but the Init passed here
  // (which contains references to early outlining variables that do not exist)
  // must be processed to use the source object address passed in as a
  // parameter. Since we need to create new AST objects in the generated
  // routine we need to create a FunctionDecl to act as the DeclContext.

  IdentifierInfo *II = &CGM.getContext().Idents.get(OutName);
  FunctionDecl *FD = FunctionDecl::Create(
      C, C.getTranslationUnitDecl(), SourceLocation(), SourceLocation(), II,
      C.VoidTy, /*TInfo=*/nullptr, SC_Static);

  QualType ObjPtrTy = C.getPointerType(Ty);

  CodeGenFunction NewCGF(CGM);
  FunctionArgList Args;
  ImplicitParamDecl DstDecl(C, FD, SourceLocation(), nullptr, ObjPtrTy);
  Args.push_back(&DstDecl);
  ImplicitParamDecl SrcDecl(C, FD, SourceLocation(), nullptr, ObjPtrTy);
  Args.push_back(&SrcDecl);

  const CGFunctionInfo &FI =
      CGM.getTypes().arrangeBuiltinFunctionDeclaration(C.VoidTy, Args);

  llvm::FunctionType *LTy = CGM.getTypes().GetFunctionType(FI);

  llvm::Function *Fn = llvm::Function::Create(
      LTy, llvm::GlobalValue::InternalLinkage, OutName, &CGM.getModule());

  CGM.SetInternalFunctionAttributes(nullptr, Fn, FI);

  NewCGF.StartFunction(FD, C.VoidTy, Fn, FI, Args);
  auto *Init = Private->getInit();
  if (Init && !NewCGF.isTrivialInitializer(Init)) {
    CodeGenFunction::RunCleanupsScope Scope(NewCGF);
    auto *CCE = cast<CXXConstructExpr>(Init);
    DeclRefExpr SrcExpr(&SrcDecl, /*RefersToEnclosingVariableOrCapture=*/false,
                        ObjPtrTy, VK_LValue, SourceLocation());
    ImplicitCastExpr CastExpr(ImplicitCastExpr::OnStack,
                              C.getPointerType(ElemType), CK_BitCast, &SrcExpr,
                              VK_RValue);
    UnaryOperator SRC(&CastExpr, UO_Deref, ElemType, VK_LValue, OK_Ordinary,
                      SourceLocation());

    QualType CTy = ElemType;
    CTy.addConst();
    ImplicitCastExpr NoOpCast(ImplicitCastExpr::OnStack, CTy, CK_NoOp, &SRC,
                              VK_LValue);

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
    NewCGF.EmitAnyExprToMem(RebuiltCCE, ArgLVal.getAddress(),
                         Ty.getQualifiers(), /*IsInitializer=*/true);
  }
  NewCGF.FinishFunction();

  return Fn;
}

llvm::Value *OpenMPCodeOutliner::emitIntelOpenMPCopyAssign(QualType Ty,
    const Expr *SrcExpr, const Expr *DstExpr, const Expr *AssignOp) {
  CodeGenModule &CGM = CGF.CGM;
  auto &C = CGM.getContext();
  QualType ElemType = Ty;
  if (Ty->isArrayType())
    ElemType = C.getBaseElementType(Ty).getNonReferenceType();

  SmallString<256> OutName;
  llvm::raw_svector_ostream Out(OutName);
  CGM.getCXXABI().getMangleContext().mangleTypeName(Ty, Out);
  Out << ".omp.copy_assign";

  if (llvm::Value *F = CGM.GetGlobalValue(OutName))
    return F;

  IdentifierInfo *II = &CGM.getContext().Idents.get(OutName);
  FunctionDecl *FD = FunctionDecl::Create(
      C, C.getTranslationUnitDecl(), SourceLocation(), SourceLocation(), II,
      C.VoidTy, /*TInfo=*/nullptr, SC_Static);

  QualType ObjPtrTy = C.getPointerType(Ty);

  CodeGenFunction NewCGF(CGM);
  FunctionArgList Args;
  ImplicitParamDecl DstDecl(C, FD, SourceLocation(), nullptr, ObjPtrTy);
  Args.push_back(&DstDecl);
  ImplicitParamDecl SrcDecl(C, FD, SourceLocation(), nullptr, ObjPtrTy);
  Args.push_back(&SrcDecl);

  const CGFunctionInfo &FI =
      CGM.getTypes().arrangeBuiltinFunctionDeclaration(C.VoidTy, Args);

  llvm::FunctionType *LTy = CGM.getTypes().GetFunctionType(FI);

  llvm::Function *Fn = llvm::Function::Create(
      LTy, llvm::GlobalValue::InternalLinkage, OutName, &CGM.getModule());

  CGM.SetInternalFunctionAttributes(nullptr, Fn, FI);

  NewCGF.StartFunction(FD, C.VoidTy, Fn, FI, Args);

  auto DestAddr = NewCGF.EmitLoadOfPointerLValue(
                                            NewCGF.GetAddrOfLocalVar(&DstDecl),
                                            ObjPtrTy->getAs<PointerType>())
                      .getAddress();

  auto SrcAddr = NewCGF.EmitLoadOfPointerLValue(
                                            NewCGF.GetAddrOfLocalVar(&SrcDecl),
                                            ObjPtrTy->getAs<PointerType>())
                     .getAddress();

  auto *SrcVD = cast<VarDecl>(cast<DeclRefExpr>(SrcExpr)->getDecl());
  auto *DestVD = cast<VarDecl>(cast<DeclRefExpr>(DstExpr)->getDecl());

  NewCGF.EmitOMPCopy(Ty, DestAddr, SrcAddr, DestVD, SrcVD, AssignOp);

  NewCGF.FinishFunction();
  return Fn;
}

namespace CGIntelOpenMP {
  /*--- Process array section expression ---*/
  /// Emit an address of the base of OMPArraySectionExpr and fills data for
  /// array sections.
  OpenMPCodeOutliner::ArraySectionDataTy
  OpenMPCodeOutliner::emitArraySectionData(const OMPArraySectionExpr *E) {
    ArraySectionDataTy Data;
    auto &C = CGF.getContext();
    if (auto *LowerBound = E->getLowerBound()) {
      Data.LowerBound = CGF.EmitScalarConversion(
          CGF.EmitScalarExpr(LowerBound), LowerBound->getType(),
          C.getSizeType(), LowerBound->getExprLoc());
    } else
      Data.LowerBound = llvm::ConstantInt::getNullValue(CGF.SizeTy);
    QualType BaseTy = OMPArraySectionExpr::getBaseOriginalType(
        E->getBase()->IgnoreParenImpCasts());
    if (auto *Length = E->getLength()) {
      Data.Length = CGF.EmitScalarConversion(CGF.EmitScalarExpr(Length),
                                             Length->getType(), C.getSizeType(),
                                             Length->getExprLoc());
    } else {
      llvm::APSInt ConstLength;
      if (auto *VAT = C.getAsVariableArrayType(BaseTy)) {
        Length = VAT->getSizeExpr();
        if (Length->isIntegerConstantExpr(ConstLength, C))
          Length = nullptr;
      } else {
        auto *CAT = C.getAsConstantArrayType(BaseTy);
        ConstLength = CAT->getSize();
      }
      llvm::Value *LengthVal;
      if (Length) {
        LengthVal = CGF.EmitScalarConversion(
            CGF.EmitScalarExpr(Length), Length->getType(), C.getSizeType(),
            Length->getExprLoc());
      } else {
        LengthVal =
            llvm::ConstantInt::get(CGF.SizeTy, ConstLength.getExtValue());
      }
      Data.Length = CGF.Builder.CreateSub(LengthVal, Data.LowerBound);
    }
    Data.Stride = llvm::ConstantInt::get(CGF.SizeTy, /*V=*/1);
    return Data;
  }
  Address OpenMPCodeOutliner::emitOMPArraySectionExpr(
                                  const OMPArraySectionExpr *E,
                                  ArraySectionTy &AS) {
    const Expr *Base = E->getBase()->IgnoreParenImpCasts();
    AS.push_back(emitArraySectionData(E));
    while (auto *ASE = dyn_cast<OMPArraySectionExpr>(Base)) {
      E = ASE;
      Base = E->getBase()->IgnoreParenImpCasts();
      AS.insert(AS.begin(), emitArraySectionData(E));
    }
    QualType BaseTy = Base->getType();
    Address BaseAddr = CGF.EmitLValue(Base).getAddress();
    if (BaseTy->isVariablyModifiedType()) {
      for (unsigned I = 0, E = AS.size(); I < E; ++I) {
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
          AS[I].VLASize = Size;
        } else {
          assert((BaseTy->isPointerType()));
          BaseTy = BaseTy->getPointeeType();
          AS[I].VLASize = llvm::ConstantPointerNull::get(CGF.VoidPtrTy);
        }
      }
    }
    return BaseAddr;
  }
  /*--- Process array section expression ---*/

  void OpenMPCodeOutliner::addArg(llvm::Value *Val) {
    BundleValues.push_back(Val);
  }

  void OpenMPCodeOutliner::addArg(StringRef Str) {
    BundleString = Str;
  }

  void OpenMPCodeOutliner::addArg(const Expr *E) {
    auto SavedIP = CGF.Builder.saveIP();
    setOutsideInsertPoint();
    if (E->getType()->isSpecificPlaceholderType(BuiltinType::OMPArraySection)) {
      ArraySectionTy AS;
      Address Base = emitOMPArraySectionExpr(
          cast<OMPArraySectionExpr>(E->IgnoreParenImpCasts()), AS);
      if (0) addArg("QUAL.OPND.ARRSECT");
      addArg(Base.getPointer());
      addArg(llvm::ConstantInt::get(CGF.SizeTy, AS.size()));
      // If VLASize of the first element is not nullptr, we have sizes for all
      // dimensions of variably modified type.
      if (0 && AS.begin()->VLASize) {
        addArg("QUAL.OPND.ARRSIZE");
        for (auto &V : AS) {
          assert(V.VLASize);
          addArg(V.VLASize);
        }
      }
      for (auto &V : AS) {
        assert(V.LowerBound);
        addArg(V.LowerBound);
        assert(V.Length);
        addArg(V.Length);
        assert(V.Stride);
        addArg(V.Stride);
      }
    } else {
      assert(E->isGLValue());
      addArg(CGF.EmitLValue(E).getPointer());
    }
    CGF.Builder.restoreIP(SavedIP);
  }

  void OpenMPCodeOutliner::getLegalDirectives(
                            SmallVector<DirectiveIntrinsicSet *, 4> &Dirs) {
    // This is likely to become complicated but for now if there are more
    // than one directive we place the clause on each if clause is allowed
    // there.
    if (Directives.size() == 1) {
      Dirs.push_back(&Directives[0]);
      return;
    }
    for (auto &D : Directives)
      if (isAllowedClauseForDirective(D.DKind, CurrentClauseKind))
        Dirs.push_back(&D);
  }

  void OpenMPCodeOutliner::startDirectiveIntrinsicSet(StringRef Begin,
                                                      StringRef End,
                                                      OpenMPDirectiveKind K) {
    assert(BundleValues.empty());
    DirectiveIntrinsicSet D(End, K);
    llvm::OperandBundleDef B(Begin, BundleValues);
    D.OpBundles.push_back(B);
    D.Intrins.push_back(llvm::Intrinsic::intel_directive);
    Directives.push_back(D);
  }

  void OpenMPCodeOutliner::emitDirective(DirectiveIntrinsicSet &D,
                                         StringRef Name) {
    assert(BundleValues.empty());
    llvm::OperandBundleDef B(Name, BundleValues);
    D.OpBundles.push_back(B);
    D.Intrins.push_back(llvm::Intrinsic::intel_directive);
    clearBundleTemps();
  }

  void OpenMPCodeOutliner::emitClause(llvm::Intrinsic::ID IID) {
    SmallVector<DirectiveIntrinsicSet *, 4> DRefs;
    getLegalDirectives(DRefs);
    for (auto *D : DRefs) {
      llvm::OperandBundleDef B(BundleString, BundleValues);
      D->OpBundles.push_back(B);
      D->Intrins.push_back(IID);
    }
    clearBundleTemps();
  }

  void OpenMPCodeOutliner::emitSimpleClause() {
    emitClause(llvm::Intrinsic::intel_directive_qual);
  }

  void OpenMPCodeOutliner::emitOpndClause() {
    emitClause(llvm::Intrinsic::intel_directive_qual_opnd);
  }

  void OpenMPCodeOutliner::emitListClause() {
    emitClause(llvm::Intrinsic::intel_directive_qual_opndlist);
  }

  void OpenMPCodeOutliner::emitImplicit(Expr *E, OpenMPClauseKind K) {
    switch (K) {
    case OMPC_private:
      addArg("QUAL.OMP.PRIVATE"); break;
    case OMPC_firstprivate:
      addArg("QUAL.OMP.FIRSTPRIVATE"); break;
    case OMPC_shared: 
      addArg("QUAL.OMP.SHARED"); break;
    default:
      llvm_unreachable("Clause not allowed");
    }
    CurrentClauseKind = K;
    addArg(E);
    emitListClause();
    CurrentClauseKind = OMPC_unknown;
  }

  void OpenMPCodeOutliner::emitImplicit(const VarDecl *VD, OpenMPClauseKind K) {
    // OMPC_unknown is used when we do not want a variable to appear in any
    // clause list, so just return when we see it.
    if (K == OMPC_unknown)
      return;

    // We don't want this DeclRefExpr to generate entries in the Def/Ref lists,
    // so temporarily save and null the CapturedStmtInfo.
    auto savedCSI = CGF.CapturedStmtInfo;
    CGF.CapturedStmtInfo = nullptr;

    DeclRefExpr DRE(const_cast<VarDecl *>(VD),
                    /*RefersToEnclosingVariableOrCapture=*/false,
                    VD->getType().getNonReferenceType(), VK_LValue,
                    SourceLocation());
    emitImplicit(&DRE, K);

    CGF.CapturedStmtInfo = savedCSI;
  }

  bool OpenMPCodeOutliner::isImplicit(const VarDecl *V) {
    return ImplicitMap.find(V) != ImplicitMap.end();
  }

  bool OpenMPCodeOutliner::isExplicit(const VarDecl *V) {
    return ExplicitRefs.find(V) != ExplicitRefs.end();
  }

  void OpenMPCodeOutliner::addImplicitClauses() {
    auto DKind = Directive.getDirectiveKind();
    if (DKind != OMPD_simd && DKind != OMPD_for &&
        DKind != OMPD_taskloop && DKind != OMPD_taskloop_simd &&
        !isOpenMPParallelDirective(DKind))
      return;

    for (const auto *VD : VarRefs) {
      if (isExplicit(VD)) continue;
      if (isImplicit(VD)) {
        emitImplicit(VD, ImplicitMap[VD]);
        continue;
      }
      if (VarDefs.find(VD) != VarDefs.end()) {
        // Defined in the region: private
        emitImplicit(VD, OMPC_private);
      } else if (DKind != OMPD_simd && DKind != OMPD_for) {
        // Referenced but not defined in the region: shared
        emitImplicit(VD, OMPC_shared);
      }
    }
  }

  void OpenMPCodeOutliner::addRefsToOuter() {
    if (CGF.CapturedStmtInfo) {
      for (const auto *VD : VarDefs) {
        if (isImplicit(VD)) continue;
        CGF.CapturedStmtInfo->recordVariableDefinition(VD);
      }
      for (const auto *VD : VarRefs) {
        if (isImplicit(VD)) continue;
        CGF.CapturedStmtInfo->recordVariableReference(VD);
      }
      for (const auto *VD : ExplicitRefs) {
        CGF.CapturedStmtInfo->recordVariableReference(VD);
      }
    }
  }

  void OpenMPCodeOutliner::addExplicit(const Expr *E) { 
    if (E->getType()->isSpecificPlaceholderType(BuiltinType::OMPArraySection)) {
      auto AE = cast<OMPArraySectionExpr>(E->IgnoreParenImpCasts());
      const Expr *Base = AE->getBase()->IgnoreParenImpCasts();
      while (auto *ASE = dyn_cast<OMPArraySectionExpr>(Base)) {
        AE = ASE;
        Base = AE->getBase()->IgnoreParenImpCasts();
      }
      E = Base;
    }
    auto *PVD = cast<VarDecl>(cast<DeclRefExpr>(E)->getDecl());
    addExplicit(PVD);
  }

  void OpenMPCodeOutliner::emitOMPSharedClause(const OMPSharedClause *Cl) {
    addArg("QUAL.OMP.SHARED");
    for (auto *E : Cl->varlists())
      addArg(E);
    emitListClause();
  }
  void OpenMPCodeOutliner::emitOMPPrivateClause(const OMPPrivateClause *Cl) {
    auto IPriv = Cl->private_copies().begin();
    for (auto *E : Cl->varlists()) {
      auto *PVD = cast<VarDecl>(cast<DeclRefExpr>(E)->getDecl());
      addExplicit(PVD);
      auto *Private = cast<VarDecl>(cast<DeclRefExpr>(*IPriv)->getDecl());
      auto *Init = Private->getInit();
      if (Init || Private->getType().isDestructedType())
        addArg("QUAL.OMP.PRIVATE:NONPOD");
      else
        addArg("QUAL.OMP.PRIVATE");
      addArg(E);
      if (Init || Private->getType().isDestructedType()) {
        addArg(emitIntelOpenMPDefaultConstructor(*IPriv));
        addArg(emitIntelOpenMPDestructor(Private->getType()));
      }
      ++IPriv;
      emitListClause();
    }
  }
  void OpenMPCodeOutliner::emitOMPLastprivateClause(
                                     const OMPLastprivateClause *Cl) {
    auto IPriv = Cl->private_copies().begin();
    auto ISrcExpr = Cl->source_exprs().begin();
    auto IDestExpr = Cl->destination_exprs().begin();
    auto IAssignOp = Cl->assignment_ops().begin();
    for (auto *E : Cl->varlists()) {
      auto *PVD = cast<VarDecl>(cast<DeclRefExpr>(E)->getDecl());
      addExplicit(PVD);
      bool IsPODType = E->getType().isPODType(CGF.getContext());
      if (!IsPODType)
        addArg("QUAL.OMP.LASTPRIVATE:NONPOD");
      else if (Cl->isConditional())
        addArg("QUAL.OMP.LASTPRIVATE:CONDITIONAL");
      else
        addArg("QUAL.OMP.LASTPRIVATE");
      addArg(E);
      if (!IsPODType) {
        addArg(emitIntelOpenMPDefaultConstructor(*IPriv));
        addArg(emitIntelOpenMPCopyAssign(E->getType(), *ISrcExpr, *IDestExpr,
                                         *IAssignOp));
        addArg(emitIntelOpenMPDestructor(E->getType()));
      }
      ++IPriv;
      ++ISrcExpr;
      ++IDestExpr;
      ++IAssignOp;
      emitListClause();
    }
  }
  void OpenMPCodeOutliner::emitOMPLinearClause(const OMPLinearClause *Cl) {
    addArg("QUAL.OMP.LINEAR");
    auto SavedIP = CGF.Builder.saveIP();
    setOutsideInsertPoint();
    for (auto *E : Cl->varlists())
      addArg(E);
    addArg(Cl->getStep() ? CGF.EmitScalarExpr(Cl->getStep())
                         : CGF.Builder.getInt32(1));
    CGF.Builder.restoreIP(SavedIP);
    emitListClause();
  }
  void OpenMPCodeOutliner::emitOMPReductionClause(
                                      const OMPReductionClause *Cl) {
    SmallString<64> Op;
    OverloadedOperatorKind OOK =
        Cl->getNameInfo().getName().getCXXOverloadedOperator();
    auto I = Cl->reduction_ops().begin();
    for (auto *E : Cl->varlists()) {
      addExplicit(E);
      assert(isa<BinaryOperator>((*I)->IgnoreImpCasts()));
      switch (OOK) {
      case OO_Plus:
        Op = "QUAL.OMP.REDUCTION.ADD";
        break;
      case OO_Minus:
        Op = "QUAL.OMP.REDUCTION.SUB";
        break;
      case OO_Star:
        Op = "QUAL.OMP.REDUCTION.MUL";
        break;
      case OO_Amp:
        Op = "QUAL.OMP.REDUCTION.BAND";
        break;
      case OO_Pipe:
        Op = "QUAL.OMP.REDUCTION.BOR";
        break;
      case OO_Caret:
        Op = "QUAL.OMP.REDUCTION.BXOR";
        break;
      case OO_AmpAmp:
        Op = "QUAL.OMP.REDUCTION.AND";
        break;
      case OO_PipePipe:
        Op = "QUAL.OMP.REDUCTION.OR";
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
        if (auto II = Cl->getNameInfo().getName().getAsIdentifierInfo()) {
          if (II->isStr("max"))
            Op = "QUAL.OMP.REDUCTION.MAX";
          else if (II->isStr("min"))
            Op = "QUAL.OMP.REDUCTION.MIN";
          QualType ElemType = E->getType();
          if (ElemType->isArrayType())
            ElemType = CGF.CGM.getContext().getBaseElementType(ElemType)
                .getNonReferenceType();
          if (ElemType->isVectorType())
            ElemType = ElemType->getAs<VectorType>()->getElementType();
          if (ElemType->isUnsignedIntegerType())
            Op += ":UNSIGNED";
        }
        break;
      }
      if (E->getType()->isSpecificPlaceholderType(BuiltinType::OMPArraySection))
        Op += ":ARRSECT";
      addArg(Op);
      addArg(E);
      emitListClause();
      ++I;
    }
  }

  void OpenMPCodeOutliner::emitOMPOrderedClause(const OMPOrderedClause *C) {
    addArg("QUAL.OMP.ORDERED");
    auto SavedIP = CGF.Builder.saveIP();
    setOutsideInsertPoint();
    if (auto *E = C->getNumForLoops())
      addArg(CGF.EmitScalarExpr(E));
    else
      addArg(CGF.Builder.getInt32(1));
    CGF.Builder.restoreIP(SavedIP);
    emitOpndClause();
  }

  void OpenMPCodeOutliner::emitOMPMapClause(const OMPMapClause *Cl) {
    StringRef Op;
    switch (Cl->getMapType()) {
    case OMPC_MAP_alloc:
      Op = "QUAL.OMP.MAP.ALLOC";
      break;
    case OMPC_MAP_to:
      Op = "QUAL.OMP.MAP.TO";
      break;
    case OMPC_MAP_from:
      Op = "QUAL.OMP.MAP.FROM";
      break;
    case OMPC_MAP_tofrom:
    case OMPC_MAP_unknown:
      Op = "QUAL.OMP.MAP.TOFROM";
      break;
    case OMPC_MAP_delete:
      Op = "QUAL.OMP.MAP.DELETE";
      break;
    case OMPC_MAP_release:
      Op = "QUAL.OMP.MAP.RELEASE";
      break;
    case OMPC_MAP_always:
      llvm_unreachable("Unexpected mapping type");
    }
    addArg(Op);
    for (auto *E : Cl->varlists())
      addArg(E);
    emitListClause();
  }

  void OpenMPCodeOutliner::emitOMPScheduleClause(const OMPScheduleClause *C) {
    int DefaultChunkSize = 0;
    SmallString<64> SchedString;
    switch (C->getScheduleKind()) {
    case OMPC_SCHEDULE_static:
      SchedString = "QUAL.OMP.SCHEDULE.STATIC";
      break;
    case OMPC_SCHEDULE_dynamic:
      DefaultChunkSize = 1;
      SchedString = "QUAL.OMP.SCHEDULE.DYNAMIC";
      break;
    case OMPC_SCHEDULE_guided:
      DefaultChunkSize = 1;
      SchedString = "QUAL.OMP.SCHEDULE.GUIDED";
      break;
    case OMPC_SCHEDULE_auto:
      SchedString = "QUAL.OMP.SCHEDULE.AUTO";
      break;
    case OMPC_SCHEDULE_runtime:
      SchedString = "QUAL.OMP.SCHEDULE.RUNTIME";
      break;
    case OMPC_SCHEDULE_unknown:
      llvm_unreachable("Unknown schedule clause");
    }

    SmallString<64> Modifiers;
    for (int Count = 0; Count < 2; ++Count) {
      SmallString<64> LocalModifier;
      auto Mod = Count == 0 ? C->getFirstScheduleModifier()
                            : C->getSecondScheduleModifier();
      switch (Mod) {
      case OMPC_SCHEDULE_MODIFIER_monotonic:
        LocalModifier = "MONOTONIC";
        break;
      case OMPC_SCHEDULE_MODIFIER_nonmonotonic:
        LocalModifier = "NONMONOTONIC";
        break;
      case OMPC_SCHEDULE_MODIFIER_simd:
        LocalModifier = "SIMD";
        break;
      case OMPC_SCHEDULE_MODIFIER_last:
      case OMPC_SCHEDULE_MODIFIER_unknown:
        break;
      }
      if (!LocalModifier.empty()) {
        if (!Modifiers.empty())
          Modifiers += ".";
        Modifiers += LocalModifier;
      }
    }
    if (!Modifiers.empty()) {
      SchedString += ':';
      SchedString += Modifiers;
    }
    addArg(SchedString);
    auto SavedIP = CGF.Builder.saveIP();
    setOutsideInsertPoint();
    if (auto *E = C->getChunkSize())
      addArg(CGF.EmitScalarExpr(E));
    else
      addArg(CGF.Builder.getInt32(DefaultChunkSize));
    CGF.Builder.restoreIP(SavedIP);
    emitListClause();
  }

  void OpenMPCodeOutliner::emitOMPFirstprivateClause(
                                  const OMPFirstprivateClause *Cl) {
    auto *IPriv = Cl->private_copies().begin();
    for (auto *E : Cl->varlists()) {
      auto *PVD = cast<VarDecl>(cast<DeclRefExpr>(E)->getDecl());
      addExplicit(PVD);
      bool IsPODType = E->getType().isPODType(CGF.getContext());
      if (!IsPODType)
        addArg("QUAL.OMP.FIRSTPRIVATE:NONPOD");
      else
        addArg("QUAL.OMP.FIRSTPRIVATE");
      addArg(E);
      if (!IsPODType) {
        addArg(emitIntelOpenMPCopyConstructor(*IPriv));
        addArg(emitIntelOpenMPDestructor(E->getType()));
      }
      ++IPriv;
      emitListClause();
    }
  }

  void OpenMPCodeOutliner::emitOMPCopyinClause(const OMPCopyinClause *Cl) {
    addArg("QUAL.OMP.COPYIN");
    auto SavedIP = CGF.Builder.saveIP();
    setOutsideInsertPoint();
    for (auto *E : Cl->varlists()) {
      if (!E->getType().isPODType(CGF.getContext()))
        CGF.CGM.ErrorUnsupported(E, "non-POD copyin variable");
      auto *PVD = cast<VarDecl>(cast<DeclRefExpr>(E)->getDecl());
      addExplicit(PVD);
      addArg(E);
    }
    CGF.Builder.restoreIP(SavedIP);
    emitListClause();
  }

  void OpenMPCodeOutliner::emitOMPIfClause(const OMPIfClause *Cl) {
    addArg("QUAL.OMP.IF");
    auto SavedIP = CGF.Builder.saveIP();
    setOutsideInsertPoint();
    addArg(CGF.EmitScalarExpr(Cl->getCondition()));
    CGF.Builder.restoreIP(SavedIP);
    emitOpndClause();
  }

  void OpenMPCodeOutliner::emitOMPNumThreadsClause(
                                    const OMPNumThreadsClause *Cl) {
    addArg("QUAL.OMP.NUM_THREADS");
    auto SavedIP = CGF.Builder.saveIP();
    setOutsideInsertPoint();
    addArg(CGF.EmitScalarExpr(Cl->getNumThreads()));
    CGF.Builder.restoreIP(SavedIP);
    emitOpndClause();
  }

  void OpenMPCodeOutliner::emitOMPDefaultClause(const OMPDefaultClause *Cl) {
    switch (Cl->getDefaultKind()) {
    case OMPC_DEFAULT_none:
      addArg("QUAL.OMP.DEFAULT.NONE");
      break;
    case OMPC_DEFAULT_shared:
      addArg("QUAL.OMP.DEFAULT.SHARED");
      break;
    case OMPC_DEFAULT_unknown:
      llvm_unreachable("Unknown default clause");
    }
    emitSimpleClause();
  }

  void OpenMPCodeOutliner::emitOMPProcBindClause(const OMPProcBindClause *Cl) {
    switch (Cl->getProcBindKind()) {
    case OMPC_PROC_BIND_master:
      addArg("QUAL.OMP.PROCBIND.MASTER");
      break;
    case OMPC_PROC_BIND_close:
      addArg("QUAL.OMP.PROCBIND.CLOSE");
      break;
    case OMPC_PROC_BIND_spread:
      addArg("QUAL.OMP.PROCBIND.SPREAD");
      break;
    case OMPC_PROC_BIND_unknown:
      llvm_unreachable("Unknown proc_bind clause");
    }
    emitSimpleClause();
  }

  void OpenMPCodeOutliner::emitOMPSafelenClause(const OMPSafelenClause *Cl) {
    addArg("QUAL.OMP.SAFELEN");
    auto SavedIP = CGF.Builder.saveIP();
    setOutsideInsertPoint();
    addArg(CGF.EmitScalarExpr(Cl->getSafelen()));
    CGF.Builder.restoreIP(SavedIP);
    emitOpndClause();
  }

  void OpenMPCodeOutliner::emitOMPSimdlenClause(const OMPSimdlenClause *Cl) {
    addArg("QUAL.OMP.SIMDLEN");
    auto SavedIP = CGF.Builder.saveIP();
    setOutsideInsertPoint();
    addArg(CGF.EmitScalarExpr(Cl->getSimdlen()));
    CGF.Builder.restoreIP(SavedIP);
    emitOpndClause();
  }

  void OpenMPCodeOutliner::emitOMPCollapseClause(const OMPCollapseClause *Cl) {
    addArg("QUAL.OMP.COLLAPSE");
    auto SavedIP = CGF.Builder.saveIP();
    setOutsideInsertPoint();
    addArg(CGF.EmitScalarExpr(Cl->getNumForLoops()));
    CGF.Builder.restoreIP(SavedIP);
    emitOpndClause();
  }

  void OpenMPCodeOutliner::emitOMPAlignedClause(const OMPAlignedClause *Cl) {
    addArg("QUAL.OMP.ALIGNED");
    auto SavedIP = CGF.Builder.saveIP();
    setOutsideInsertPoint();
    for (auto *E : Cl->varlists())
      addArg(E);
    addArg(Cl->getAlignment() ? CGF.EmitScalarExpr(Cl->getAlignment())
                              : CGF.Builder.getInt32(0));
    CGF.Builder.restoreIP(SavedIP);
    emitListClause();
  }

  void
  OpenMPCodeOutliner::emitOMPGrainsizeClause(const OMPGrainsizeClause *Cl) {
    addArg("QUAL.OMP.GRAINSIZE");
    auto SavedIP = CGF.Builder.saveIP();
    setOutsideInsertPoint();
    addArg(CGF.EmitScalarExpr(Cl->getGrainsize()));
    CGF.Builder.restoreIP(SavedIP);
    emitOpndClause();
  }

  void OpenMPCodeOutliner::emitOMPNumTasksClause(const OMPNumTasksClause *Cl) {
    addArg("QUAL.OMP.NUM_TASKS");
    auto SavedIP = CGF.Builder.saveIP();
    setOutsideInsertPoint();
    addArg(CGF.EmitScalarExpr(Cl->getNumTasks()));
    CGF.Builder.restoreIP(SavedIP);
    emitOpndClause();
  }

  void OpenMPCodeOutliner::emitOMPPriorityClause(const OMPPriorityClause *Cl) {
    addArg("QUAL.OMP.PRIORITY");
    auto SavedIP = CGF.Builder.saveIP();
    setOutsideInsertPoint();
    addArg(CGF.EmitScalarExpr(Cl->getPriority()));
    CGF.Builder.restoreIP(SavedIP);
    emitOpndClause();
  }

  void OpenMPCodeOutliner::emitOMPFinalClause(const OMPFinalClause *Cl) {
    addArg("QUAL.OMP.FINAL");
    auto SavedIP = CGF.Builder.saveIP();
    setOutsideInsertPoint();
    addArg(CGF.EmitScalarExpr(Cl->getCondition()));
    CGF.Builder.restoreIP(SavedIP);
    emitOpndClause();
  }

  void OpenMPCodeOutliner::emitOMPNogroupClause(const OMPNogroupClause *) {
    addArg("QUAL.OMP.NOGROUP");
    emitSimpleClause();
  }

  void OpenMPCodeOutliner::emitOMPMergeableClause(const OMPMergeableClause *) {
    addArg("QUAL.OMP.MERGEABLE");
    emitSimpleClause();
  }

  void OpenMPCodeOutliner::emitOMPUntiedClause(const OMPUntiedClause *) {
    addArg("QUAL.OMP.UNTIED");
    emitSimpleClause();
  }

  void OpenMPCodeOutliner::emitOMPCopyprivateClause(
                                        const OMPCopyprivateClause *) {}
  void OpenMPCodeOutliner::emitOMPNowaitClause(const OMPNowaitClause *) {}
  void OpenMPCodeOutliner::emitOMPFlushClause(const OMPFlushClause *) {}
  void OpenMPCodeOutliner::emitOMPReadClause(const OMPReadClause *) {}
  void OpenMPCodeOutliner::emitOMPWriteClause(const OMPWriteClause *) {}
  void OpenMPCodeOutliner::emitOMPUpdateClause(const OMPUpdateClause *) {}
  void OpenMPCodeOutliner::emitOMPCaptureClause(const OMPCaptureClause *) {}
  void OpenMPCodeOutliner::emitOMPSeqCstClause(const OMPSeqCstClause *) {}
  void OpenMPCodeOutliner::emitOMPDependClause(const OMPDependClause *) {}
  void OpenMPCodeOutliner::emitOMPDeviceClause(const OMPDeviceClause *) {}
  void OpenMPCodeOutliner::emitOMPThreadsClause(const OMPThreadsClause *) {}
  void OpenMPCodeOutliner::emitOMPSIMDClause(const OMPSIMDClause *) {}
  void OpenMPCodeOutliner::emitOMPNumTeamsClause(const OMPNumTeamsClause *) {}
  void OpenMPCodeOutliner::emitOMPThreadLimitClause(
                                              const OMPThreadLimitClause *) {}
  void OpenMPCodeOutliner::emitOMPHintClause(const OMPHintClause *) {}
  void OpenMPCodeOutliner::emitOMPDistScheduleClause(
                                              const OMPDistScheduleClause *) {}
  void OpenMPCodeOutliner::emitOMPDefaultmapClause(
                                                const OMPDefaultmapClause *) {}
  void OpenMPCodeOutliner::emitOMPToClause(const OMPToClause *) {}
  void OpenMPCodeOutliner::emitOMPFromClause(const OMPFromClause *) {}
  void OpenMPCodeOutliner::emitOMPUseDevicePtrClause(
                                              const OMPUseDevicePtrClause *) {}
  void OpenMPCodeOutliner::emitOMPIsDevicePtrClause(
                                               const OMPIsDevicePtrClause *) {}

  OpenMPCodeOutliner::OpenMPCodeOutliner(CodeGenFunction &CGF,
                                         const OMPExecutableDirective &D)
      : CGF(CGF), C(CGF.CGM.getLLVMContext()), Directive(D) {
    RegionEntryDirective = CGF.CGM.getIntrinsic(
                               llvm::Intrinsic::directive_region_entry);
    RegionExitDirective = CGF.CGM.getIntrinsic(
                               llvm::Intrinsic::directive_region_exit);
    
    // Create a marker call at the start of the region.  The values generated
    // from clauses must be inserted before this point.
    SmallVector<llvm::Value*, 1> CallArgs;
    OutsideInsertInstruction = CGF.Builder.CreateCall(RegionEntryDirective,
                                                      CallArgs);

    if (auto *LoopDir = dyn_cast<OMPLoopDirective>(&D)) {
      auto DKind = LoopDir->getDirectiveKind();
      for (auto *E : LoopDir->counters()) {
        auto *PVD = cast<VarDecl>(cast<DeclRefExpr>(E)->getDecl());
        if (isOpenMPSimdDirective(DKind))
          ImplicitMap.insert(std::make_pair(PVD, OMPC_unknown));
        else
          ImplicitMap.insert(std::make_pair(PVD, OMPC_private));
      }
      auto IVExpr = cast<DeclRefExpr>(LoopDir->getIterationVariable());
      auto IVDecl = cast<VarDecl>(IVExpr->getDecl());
      ImplicitMap.insert(std::make_pair(IVDecl, OMPC_unknown));
      if (isOpenMPWorksharingDirective(DKind) ||
          isOpenMPTaskLoopDirective(DKind) ||
          isOpenMPDistributeDirective(DKind)) {
        auto LBExpr = cast<DeclRefExpr>(LoopDir->getLowerBoundVariable());
        auto LBDecl = cast<VarDecl>(LBExpr->getDecl());
        ImplicitMap.insert(std::make_pair(LBDecl, OMPC_firstprivate));
        auto UBExpr = cast<DeclRefExpr>(LoopDir->getUpperBoundVariable());
        auto UBDecl = cast<VarDecl>(UBExpr->getDecl());
        ImplicitMap.insert(std::make_pair(UBDecl, OMPC_firstprivate));
      }
    }
  }

  void OpenMPCodeOutliner::emitMultipleDirectives(DirectiveIntrinsicSet &D) {
    int I = 0; 
    for (auto O : D.OpBundles) {
      auto Int = D.Intrins[I];
      SmallVector<llvm::Value*, 1> CallArgs;
      CallArgs.push_back(
          llvm::MetadataAsValue::get(C, llvm::MDString::get(C, O.getTag())));
      for (auto *V : O.inputs())
        CallArgs.push_back(V);
      llvm::Function *IFunc;
      switch(Int) {
      case llvm::Intrinsic::intel_directive:
      case llvm::Intrinsic::intel_directive_qual:
      case llvm::Intrinsic::intel_directive_qual_opndlist:
        IFunc = CGF.CGM.getIntrinsic(Int);
        break;
      case llvm::Intrinsic::intel_directive_qual_opnd: {
        llvm::Type *Types[] = {CallArgs[1]->getType()};
        IFunc = CGF.CGM.getIntrinsic(Int, Types);
        break;
      }
      default:
        llvm_unreachable("Unexpected intrinsic");
      }
      CGF.EmitRuntimeCall(IFunc, CallArgs);
      I++; 
    }
  }

  OpenMPCodeOutliner::~OpenMPCodeOutliner() {
    addImplicitClauses();

    // Insert the start directives
    auto EndIP = CGF.Builder.saveIP();
    setOutsideInsertPoint();
    for (auto I = Directives.begin(), E = Directives.end(); I != E; ++I) {
      auto &D = *I;
      if (CGF.getLangOpts().IntelOpenMPRegion) {
        SmallVector<llvm::Value*, 1> CallArgs;
        D.CallEntry = CGF.Builder.CreateCall(RegionEntryDirective, CallArgs,
                                             D.OpBundles);
        D.CallEntry->setCallingConv(CGF.getRuntimeCC());
      } else {
        emitDirective(D, "DIR.QUAL.LIST.END");
        emitMultipleDirectives(D);
      }
      D.clear();
      // Place the end directive in place of the start
      emitDirective(D, D.End);
      if (!CGF.getLangOpts().IntelOpenMPRegion)
        emitDirective(D, "DIR.QUAL.LIST.END");
    }
    CGF.Builder.restoreIP(EndIP);

    // Now emit the end directives
    for (auto I = Directives.rbegin(), E = Directives.rend(); I != E; ++I) {
      auto &D = *I;
      if (CGF.getLangOpts().IntelOpenMPRegion) {
        SmallVector<llvm::Value*, 1> CallArgs;
        CallArgs.push_back(D.CallEntry);
        auto *CallExit = CGF.Builder.CreateCall(RegionExitDirective, CallArgs,
                                                D.OpBundles);
        CallExit->setCallingConv(CGF.getRuntimeCC());
      } else {
        emitMultipleDirectives(D);
      }
    }

    addRefsToOuter();
    OutsideInsertInstruction->eraseFromParent();
  }

  void OpenMPCodeOutliner::emitOMPParallelDirective() {
    startDirectiveIntrinsicSet("DIR.OMP.PARALLEL", "DIR.OMP.END.PARALLEL");
  }
  void OpenMPCodeOutliner::emitOMPParallelForDirective() {
    startDirectiveIntrinsicSet("DIR.OMP.PARALLEL.LOOP",
                               "DIR.OMP.END.PARALLEL.LOOP");
  }
  void OpenMPCodeOutliner::emitOMPSIMDDirective() {
    startDirectiveIntrinsicSet("DIR.OMP.SIMD", "DIR.OMP.END.SIMD");
  }

  void OpenMPCodeOutliner::emitOMPForDirective() {
    startDirectiveIntrinsicSet("DIR.OMP.LOOP", "DIR.OMP.END.LOOP");
  }

  void OpenMPCodeOutliner::emitOMPParallelForSimdDirective() {
    startDirectiveIntrinsicSet("DIR.OMP.PARALLEL.LOOP",
                               "DIR.OMP.END.PARALLEL.LOOP", OMPD_parallel_for);
    startDirectiveIntrinsicSet("DIR.OMP.SIMD", "DIR.OMP.END.SIMD", OMPD_simd);
  }

  void OpenMPCodeOutliner::emitOMPTaskLoopDirective() {
    startDirectiveIntrinsicSet("DIR.OMP.TASKLOOP",
                               "DIR.OMP.END.TASKLOOP", OMPD_taskloop);
  }

  void OpenMPCodeOutliner::emitOMPTaskLoopSimdDirective() {
    startDirectiveIntrinsicSet("DIR.OMP.TASKLOOP",
                               "DIR.OMP.END.TASKLOOP", OMPD_taskloop);
    startDirectiveIntrinsicSet("DIR.OMP.SIMD", "DIR.OMP.END.SIMD", OMPD_simd);
  }

  void OpenMPCodeOutliner::emitOMPAtomicDirective(OMPAtomicClause ClauseKind) {
    startDirectiveIntrinsicSet("DIR.OMP.ATOMIC", "DIR.OMP.END.ATOMIC");

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
    addArg(Op);
    emitSimpleClause();
  }
  void OpenMPCodeOutliner::emitOMPSingleDirective() {
    startDirectiveIntrinsicSet("DIR.OMP.SINGLE", "DIR.OMP.END.SINGLE");
  }
  void OpenMPCodeOutliner::emitOMPMasterDirective() {
    startDirectiveIntrinsicSet("DIR.OMP.MASTER", "DIR.OMP.END.MASTER");
  }
  void OpenMPCodeOutliner::emitOMPCriticalDirective() {
    startDirectiveIntrinsicSet("DIR.OMP.CRITICAL", "DIR.OMP.END.CRITICAL");
  }
  void OpenMPCodeOutliner::emitOMPOrderedDirective() {
    startDirectiveIntrinsicSet("DIR.OMP.ORDERED", "DIR.OMP.END.ORDERED");
  }
  void OpenMPCodeOutliner::emitOMPTargetDirective() {
    startDirectiveIntrinsicSet("DIR.OMP.TARGET", "DIR.OMP.END.TARGET");
  }
  OpenMPCodeOutliner &OpenMPCodeOutliner::operator<<(
                                         ArrayRef<OMPClause *> Clauses) {
    for (auto *C : Clauses) {
      CurrentClauseKind = C->getClauseKind();
      switch (CurrentClauseKind) {
#define OPENMP_CLAUSE(Name, Class)                                             \
  case OMPC_##Name:                                                            \
    emit##Class(cast<Class>(C));                                               \
    break;
#include "clang/Basic/OpenMPKinds.def"
      case OMPC_uniform:
      case OMPC_threadprivate:
      case OMPC_unknown:
        llvm_unreachable("Clause not allowed");
      }
    }
    CurrentClauseKind = OMPC_unknown;
    return *this;
  }

  /// \brief Emit the captured statement body.
  void CGOpenMPRegionInfo::EmitBody(CodeGenFunction &CGF, const Stmt *S) {
    if (!CGF.HaveInsertPoint())
      return;
    CodeGenFunction::OMPPrivateScope PrivScope(CGF);
    auto *CS = cast<CapturedStmt>(S);
    // Make sure the globals captured in the provided statement are local by
    // using the privatization logic. We assume the same variable is not
    // captured more than once.
    for (auto &C : CS->captures()) {
      if (!C.capturesVariable() && !C.capturesVariableByCopy())
        continue;

      const VarDecl *VD = C.getCapturedVar();
      if (VD->isLocalVarDeclOrParm())
        continue;

      DeclRefExpr DRE(const_cast<VarDecl *>(VD),
                      /*RefersToEnclosingVariableOrCapture=*/false,
                      VD->getType().getNonReferenceType(), VK_LValue,
                      SourceLocation());
      PrivScope.addPrivate(VD, [&CGF, &DRE]() -> Address {
        return CGF.EmitLValue(&DRE).getAddress();
      });
    }
    // 'private' clause must be handled separately.
    if (D.hasClausesOfKind<OMPPrivateClause>()) {
      for (const auto *C : D.getClausesOfKind<OMPPrivateClause>()) {
        for (auto *Ref : C->varlists()) {
          if (auto *DRE = dyn_cast<DeclRefExpr>(Ref->IgnoreParenImpCasts())) {
            if (auto *VD = dyn_cast<VarDecl>(DRE->getDecl())) {
              if (VD->isLocalVarDeclOrParm())
                continue;

              DeclRefExpr DRE(const_cast<VarDecl *>(VD),
                              /*RefersToEnclosingVariableOrCapture=*/false,
                              VD->getType().getNonReferenceType(), VK_LValue,
                              SourceLocation());
              PrivScope.addPrivate(VD, [&CGF, &DRE]() -> Address {
                return CGF.EmitLValue(&DRE).getAddress();
              });
            }
          }
        }
      }
    }
    (void)PrivScope.Privatize();
    CGF.EmitStmt(CS->getCapturedStmt());
  }

  // \brief Retrieve the value of the context parameter.
  llvm::Value *CGOpenMPRegionInfo::getContextValue() const {
    if (OldCSI)
      return OldCSI->getContextValue();
    llvm_unreachable("No context value for inlined OpenMP region");
  }
  void CGOpenMPRegionInfo::setContextValue(llvm::Value *V) {
    if (OldCSI) {
      OldCSI->setContextValue(V);
      return;
    }
    llvm_unreachable("No context value for inlined OpenMP region");
  }
  /// \brief Lookup the captured field decl for a variable.
  const FieldDecl *CGOpenMPRegionInfo::lookup(
                                   const VarDecl *VD) const {
    if (OldCSI)
      return OldCSI->lookup(VD);
    // If there is no outer outlined region,no need to lookup in a list of
    // captured variables, we can use the original one.
    return nullptr;
  }
  FieldDecl *CGOpenMPRegionInfo::getThisFieldDecl() const {
    if (OldCSI)
      return OldCSI->getThisFieldDecl();
    return nullptr;
  }

} // namespace

static
void emitPreInitStmt(CodeGenFunction &CGF, const OMPExecutableDirective &S) {
  for (const auto *C : S.clauses()) {
    if (auto *CPI = OMPClauseWithPreInit::get(C)) {
      if (auto *PreInit = cast_or_null<DeclStmt>(CPI->getPreInitStmt())) {
        for (const auto *I : PreInit->decls()) {
          if (!I->hasAttr<OMPCaptureNoInitAttr>())
            CGF.EmitVarDecl(cast<VarDecl>(*I));
          else {
            CodeGenFunction::AutoVarEmission Emission =
                CGF.EmitAutoVarAlloca(cast<VarDecl>(*I));
            CGF.EmitAutoVarCleanups(Emission);
          }
        }
      }
    }
  }
}

void CodeGenFunction::EmitIntelOpenMPDirective(
    const OMPExecutableDirective &S) {
  OpenMPCodeOutliner Outliner(*this, S);

  // We don't want to emit private clauses for counters in regular loops.
  // Add to explicit to prevent that happening via the implicit rules.
  if (auto *LoopDir = dyn_cast<OMPLoopDirective>(&S)) {
    for (auto *E : LoopDir->counters()) {
      auto *PVD = cast<VarDecl>(cast<DeclRefExpr>(E)->getDecl());
      Outliner.addExplicit(PVD);
    }
  }
  auto SavedIP = Builder.saveIP();
  Outliner.setOutsideInsertPoint();
  emitPreInitStmt(*this, S);
  Builder.restoreIP(SavedIP);

  switch (S.getDirectiveKind()) {
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
  case OMPD_critical:
    Outliner.emitOMPCriticalDirective();
    break;
  case OMPD_ordered:
    Outliner.emitOMPOrderedDirective();
    break;
  case OMPD_target:
    CGM.setHasTargetCode();
    Outliner.emitOMPTargetDirective();
    break;
  case OMPD_task:
  case OMPD_sections:
  case OMPD_section:
  case OMPD_taskyield:
  case OMPD_barrier:
  case OMPD_taskwait:
  case OMPD_taskgroup:
  case OMPD_flush:
  case OMPD_teams:
  case OMPD_teams_distribute:
  case OMPD_teams_distribute_simd:
  case OMPD_cancel:
  case OMPD_target_data:
  case OMPD_parallel_sections:
  case OMPD_for_simd:
  case OMPD_cancellation_point:
  case OMPD_distribute:
  case OMPD_target_enter_data:
  case OMPD_target_exit_data:
  case OMPD_target_parallel:
  case OMPD_target_parallel_for:
  case OMPD_target_parallel_for_simd:
  case OMPD_target_simd:
  case OMPD_target_update:
  case OMPD_distribute_parallel_for:
  case OMPD_distribute_parallel_for_simd:
  case OMPD_distribute_simd:
    break;
  case OMPD_declare_target:
  case OMPD_end_declare_target:
  case OMPD_threadprivate:
  case OMPD_declare_reduction:
  case OMPD_declare_simd:
  case OMPD_unknown:
    llvm_unreachable("Wrong OpenMP directive");
  case OMPD_simd:
  case OMPD_for:
  case OMPD_parallel_for:
  case OMPD_parallel_for_simd:
  case OMPD_taskloop:
  case OMPD_taskloop_simd:
    llvm_unreachable("OpenMP loops not handled here");
  }
  Outliner << S.clauses();
  if (S.hasAssociatedStmt()) {
    InlinedOpenMPRegionRAII Region(*this, Outliner, S);
    CapturedStmtInfo->EmitBody(*this, S.getAssociatedStmt());
  }
}
#endif // INTEL_SPECIFIC_OPENMP
