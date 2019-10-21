#if INTEL_COLLAB                                           // -*- C++ -*-
//===--- CGOpenMPLateOutline.cpp - OpenMP Late-Outlining ------*- C++ -*---===//
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
using namespace clang;
using namespace CodeGen;

llvm::Value *
OpenMPLateOutliner::emitOpenMPDefaultConstructor(const Expr *IPriv) {

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
                        /*Id=*/nullptr, PtrTy, ImplicitParamDecl::Other);
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
OpenMPLateOutliner::emitOpenMPDestructor(QualType Ty) {
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
                        /*Id=*/nullptr, PtrTy, ImplicitParamDecl::Other);
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
OpenMPLateOutliner::emitOpenMPCopyConstructor(const Expr *IPriv) {
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
      FunctionTy, C.getTrivialTypeSourceInfo(FunctionTy), SC_Static);

  QualType ObjPtrTy = C.getPointerType(Ty);

  CodeGenFunction NewCGF(CGM);
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
    auto *CCE = cast<CXXConstructExpr>(Init);
    DeclRefExpr SrcExpr(C, &SrcDecl,
                        /*RefersToEnclosingVariableOrCapture=*/false,
                        ObjPtrTy, VK_LValue, SourceLocation());
    ImplicitCastExpr CastExpr(ImplicitCastExpr::OnStack,
                              C.getPointerType(ElemType), CK_BitCast, &SrcExpr,
                              VK_RValue);
    UnaryOperator SRC(&CastExpr, UO_Deref, ElemType, VK_LValue, OK_Ordinary,
                      SourceLocation(), /*CanOverflow=*/false);

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

llvm::Value *OpenMPLateOutliner::emitOpenMPCopyAssign(QualType Ty,
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
  FunctionProtoType::ExtProtoInfo EPI;
  QualType FunctionTy = C.getFunctionType(C.VoidTy, llvm::None, EPI);
  FunctionDecl *FD = FunctionDecl::Create(
      C, C.getTranslationUnitDecl(), SourceLocation(), SourceLocation(), II,
      FunctionTy, C.getTrivialTypeSourceInfo(FunctionTy), SC_Static);

  QualType ObjPtrTy = C.getPointerType(Ty);

  CodeGenFunction NewCGF(CGM);
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
      LengthVal = CGF.EmitScalarConversion(CGF.EmitScalarExpr(Length),
                                           Length->getType(), C.getSizeType(),
                                           Length->getExprLoc());
    } else {
      LengthVal = llvm::ConstantInt::get(CGF.SizeTy, ConstLength.getExtValue());
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
  Address BaseAddr = CGF.EmitLValue(Base).getAddress();
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

void OpenMPLateOutliner::addArg(llvm::Value *V, bool Handled) {
  BundleValues.push_back(V);
  if (Handled)
    HandledValues.insert(V);
}

void OpenMPLateOutliner::addArg(StringRef Str) { BundleString = Str; }

void OpenMPLateOutliner::addArg(const Expr *E, bool IsRef) {
  if (isa<ArraySubscriptExpr>(E->IgnoreParenImpCasts()) ||
      E->getType()->isSpecificPlaceholderType(BuiltinType::OMPArraySection)) {
    ArraySectionTy AS;
    Address Base = emitOMPArraySectionExpr(E, &AS);
    llvm::Value *V = Base.getPointer();
    if (IsRef) {
      auto *LI = dyn_cast<llvm::LoadInst>(V);
      assert(LI && "expected load instruction for reference type");
      V = LI->getPointerOperand();
    }
    addArg(V, /*Handled=*/true);
    addArg(llvm::ConstantInt::get(CGF.SizeTy, AS.size()));
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
    llvm::Value *V = CGF.EmitLValue(E).getPointer();
    if (IsRef) {
      auto *LI = dyn_cast<llvm::LoadInst>(V);
      assert(LI && "expected load instruction for reference type");
      V = LI->getPointerOperand();
    }
    addArg(V, /*Handled=*/true);
  }
}

void OpenMPLateOutliner::getApplicableDirectives(
    OpenMPClauseKind CK, SmallVector<DirectiveIntrinsicSet *, 4> &Dirs) {

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

    switch (CK) {
    case OMPC_reduction:
      // Prevent reduction on target which is being allowed by
      // isAllowedClauseForDirective.
      if (D.DKind != OMPD_target && isAllowedClauseForDirective(D.DKind, CK))
        Dirs.push_back(&D);
      break;
    case OMPC_unknown:
      Dirs.push_back(&D);
      break;
    default:
      if (isAllowedClauseForDirective(D.DKind, CK))
        Dirs.push_back(&D);
    }
  }
}

void OpenMPLateOutliner::startDirectiveIntrinsicSet(StringRef Begin,
                                                    StringRef End,
                                                    OpenMPDirectiveKind K) {
  assert(BundleValues.empty());
  DirectiveIntrinsicSet D(End, K);
  llvm::OperandBundleDef B(Begin, BundleValues);
  D.OpBundles.push_back(B);
  Directives.push_back(D);
}

void OpenMPLateOutliner::emitDirective(DirectiveIntrinsicSet &D,
                                       StringRef Name) {
  assert(BundleValues.empty());
  llvm::OperandBundleDef B(Name, BundleValues);
  D.OpBundles.push_back(B);
  clearBundleTemps();
}

void OpenMPLateOutliner::emitClause(OpenMPClauseKind CK) {
  SmallVector<DirectiveIntrinsicSet *, 4> DRefs;
  getApplicableDirectives(CK, DRefs);
  for (auto *D : DRefs) {
    llvm::OperandBundleDef B(BundleString, BundleValues);
    D->OpBundles.push_back(B);
  }
  clearBundleTemps();
}

void OpenMPLateOutliner::emitImplicit(Expr *E, ImplicitClauseKind K) {
  switch (K) {
  case ICK_private:
    addArg("QUAL.OMP.PRIVATE");
    break;
  case ICK_specified_firstprivate:
  case ICK_firstprivate:
    addArg("QUAL.OMP.FIRSTPRIVATE");
    break;
  case ICK_lastprivate:
    addArg("QUAL.OMP.LASTPRIVATE");
    break;
  case ICK_shared:
    addArg("QUAL.OMP.SHARED");
    break;
  case ICK_map_tofrom:
    addArg("QUAL.OMP.MAP.TOFROM");
    break;
  case ICK_normalized_iv:
    addArg("QUAL.OMP.NORMALIZED.IV");
    break;
  case ICK_normalized_ub:
    addArg("QUAL.OMP.NORMALIZED.UB");
    break;
  default:
    llvm_unreachable("Clause not allowed");
  }
  ClauseEmissionHelper CEH(*this, OMPC_unknown);
  addArg(E);
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
    if (K == ICK_normalized_iv)
      addArg("QUAL.OMP.NORMALIZED.IV");
    else
      addArg("QUAL.OMP.NORMALIZED.UB");
    for (auto &A : ImplicitMap) {
      if (A.second == K) {
        DeclRefExpr DRE(CGF.CGM.getContext(), const_cast<VarDecl *>(A.first),
                        /*RefersToEnclosingVariableOrCapture=*/false,
                        A.first->getType().getNonReferenceType(), VK_LValue,
                        SourceLocation());
        addArg(&DRE);
        A.second = ICK_unknown;
      }
    }
    return;
  }
#endif // INTEL_CUSTOMIZATION

  if (!OMPLateOutlineLexicalScope::isCapturedVar(CGF, VD)) {
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

bool OpenMPLateOutliner::isExplicit(const VarDecl *V) {
  return ExplicitRefs.find(V) != ExplicitRefs.end();
}

bool OpenMPLateOutliner::hasMapClause(const VarDecl *V) {
  return MapRefs.find(V) != MapRefs.end();
}

bool OpenMPLateOutliner::alreadyHandled(llvm::Value *V) {
  return HandledValues.find(V) != HandledValues.end();
}

void OpenMPLateOutliner::addImplicitClauses() {
  if (!isOpenMPLoopDirective(CurrentDirectiveKind) &&
      !isOpenMPParallelDirective(CurrentDirectiveKind) &&
      CurrentDirectiveKind != OMPD_task &&
      CurrentDirectiveKind != OMPD_target && CurrentDirectiveKind != OMPD_teams)
    return;

  for (const auto *VD : VarRefs) {
    if (isExplicit(VD))
      continue;
    if (isImplicit(VD)) {
      emitImplicit(VD, ImplicitMap[VD]);
      continue;
    }
    if (VarDefs.find(VD) != VarDefs.end()) {
      // Defined in the region
      if (!VD->getType()->isConstantSizeType()) {
        // An alloca inserted inside the region cannot be used on a clause.
        continue;
      } else if (VD->getStorageDuration() == SD_Static) {
        if (isAllowedClauseForDirective(CurrentDirectiveKind, OMPC_shared))
          emitImplicit(VD, ICK_shared);
      } else {
        emitImplicit(VD, ICK_private);
      }
    } else if (CurrentDirectiveKind == OMPD_target) {
      if (!VD->getType()->isScalarType() ||
          Directive.hasClausesOfKind<OMPDefaultmapClause>())
        emitImplicit(VD, ICK_map_tofrom);
      else
        emitImplicit(VD, ICK_firstprivate);
    } else if (isImplicitTask(OMPD_task)) {
      emitImplicit(VD, ICK_firstprivate);
    } else if (isAllowedClauseForDirective(CurrentDirectiveKind, OMPC_shared)) {
      // Referenced but not defined in the region: shared
      emitImplicit(VD, ICK_shared);
    }
  }
  // Definitions of temps or uses of other values without representation in
  // the AST must be added.  We try to save these while generating code but
  // must use a ValueHandle since some Values are created but are not in the
  // final IR.  This is this is temporary until the back end can handle local
  // allocas correctly.
  for (llvm::WeakTrackingVH &VH : ReferencedValues) {
    if (!VH.pointsToAliveValue())
      continue;
    llvm::Value *V = VH;
    if (V->getName().find(".omp.iv") != StringRef::npos ||
        V->getName().find(".omp.ub") != StringRef::npos)
      continue;
    bool ValueFound = false;
    for (llvm::WeakTrackingVH &VH : DefinedValues) {
      if (VH.pointsToAliveValue() && VH == V) {
        ValueFound = true;
        break;
      }
    }
    if (ValueFound) {
      // Defined in the region: private
      if (!alreadyHandled(V)) {
        ClauseEmissionHelper CEH(*this, OMPC_private);
        addArg("QUAL.OMP.PRIVATE");
        addArg(V, /*Handled=*/true);
      }
    } else if (CurrentDirectiveKind == OMPD_target) {
      if (!alreadyHandled(V)) {
        ClauseEmissionHelper CEH(*this, OMPC_firstprivate);
        addArg("QUAL.OMP.FIRSTPRIVATE");
        addArg(V, /*Handled=*/true);
      }
    } else if (isAllowedClauseForDirective(CurrentDirectiveKind, OMPC_shared)) {
      // Referenced but not defined in the region: shared
      if (!alreadyHandled(V)) {
        ClauseEmissionHelper CEH(*this, OMPC_shared);
        addArg("QUAL.OMP.SHARED");
        addArg(V, /*Handled=*/true);
      }
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
    for (const auto *VD : ExplicitRefs) {
      CGF.CapturedStmtInfo->recordVariableReference(VD);
    }
    for (llvm::WeakTrackingVH &VH : DefinedValues) {
      if (VH.pointsToAliveValue())
        CGF.CapturedStmtInfo->recordValueDefinition(VH);
    }
    for (llvm::WeakTrackingVH &VH : ReferencedValues) {
      if (VH.pointsToAliveValue())
        CGF.CapturedStmtInfo->recordValueReference(VH);
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
    const VarDecl *PVD = getExplicitVarDecl(E);
    assert(PVD && "expected VarDecl in shared clause");
    addExplicit(PVD);
    ClauseEmissionHelper CEH(*this, OMPC_shared);
    addArg("QUAL.OMP.SHARED");
    addArg(E);
  }
}

void OpenMPLateOutliner::emitOMPPrivateClause(const OMPPrivateClause *Cl) {
  auto IPriv = Cl->private_copies().begin();
  for (auto *E : Cl->varlists()) {
    const VarDecl *PVD = getExplicitVarDecl(E);
    assert(PVD && "expected VarDecl in private clause");
    addExplicit(PVD);
    bool IsCapturedExpr = isa<OMPCapturedExprDecl>(PVD);
    bool IsRef = !IsCapturedExpr && PVD->getType()->isReferenceType();
    auto *Private = cast<VarDecl>(cast<DeclRefExpr>(*IPriv)->getDecl());
    const Expr *Init = Private->getInit();
    ClauseEmissionHelper CEH(*this, OMPC_private, "QUAL.OMP.PRIVATE");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    if (Init || Private->getType().isDestructedType())
      CSB.setNonPod();
    if (IsRef)
      CSB.setByRef();
    addArg(CSB.getString());
    addArg(E, IsRef);
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
    const VarDecl *PVD = getExplicitVarDecl(E);
    assert(PVD && "expected VarDecl in lastprivate clause");
    addExplicit(PVD);
    bool IsPODType = E->getType().isPODType(CGF.getContext());
    bool IsCapturedExpr = isa<OMPCapturedExprDecl>(PVD);
    bool IsRef = !IsCapturedExpr && PVD->getType()->isReferenceType();
    ClauseEmissionHelper CEH(*this, OMPC_lastprivate, "QUAL.OMP.LASTPRIVATE");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    if (!IsPODType)
      CSB.setNonPod();
    if (IsRef)
      CSB.setByRef();
#if INTEL_CUSTOMIZATION
    if (Cl->isConditional())
      CSB.setConditional();
#endif // INTEL_CUSTOMIZATION
    addArg(CSB.getString());
    addArg(E, IsRef);
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
    const VarDecl *PVD = getExplicitVarDecl(E);
    assert(PVD && "expected VarDecl in linear clause");
    addExplicit(PVD);
    bool IsCapturedExpr = isa<OMPCapturedExprDecl>(PVD);
    bool IsRef = !IsCapturedExpr && PVD->getType()->isReferenceType();
    ClauseEmissionHelper CEH(*this, OMPC_linear, "QUAL.OMP.LINEAR");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    if (IsRef)
      CSB.setByRef();
    addArg(CSB.getString());
    addArg(E, IsRef);
    addArg(Cl->getStep() ? CGF.EmitScalarExpr(Cl->getStep())
                         : CGF.Builder.getInt32(1));
  }
}

template <typename RedClause>
void OpenMPLateOutliner::emitOMPReductionClauseCommon(const RedClause *Cl,
                                                      StringRef QualName) {
  OverloadedOperatorKind OOK =
      Cl->getNameInfo().getName().getCXXOverloadedOperator();
  auto I = Cl->reduction_ops().begin();
  for (auto *E : Cl->varlists()) {
    const VarDecl *PVD = getExplicitVarDecl(E);
    assert(PVD && "expected VarDecl in reduction clause");
    addExplicit(PVD);
    bool IsCapturedExpr = isa<OMPCapturedExprDecl>(PVD);
    bool IsRef = !IsCapturedExpr && PVD->getType()->isReferenceType();
    assert(isa<BinaryOperator>((*I)->IgnoreImpCasts()));
    ClauseEmissionHelper CEH(*this, Cl->getClauseKind(), "QUAL.OMP.");
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
      if (auto II = Cl->getNameInfo().getName().getAsIdentifierInfo()) {
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
    if (IsRef)
      CSB.setByRef();
    if (isa<ArraySubscriptExpr>(E->IgnoreParenImpCasts()) ||
        E->getType()->isSpecificPlaceholderType(BuiltinType::OMPArraySection))
      CSB.setArrSect();
    addArg(CSB.getString());
    addArg(E, IsRef);
    ++I;
  }
}

void OpenMPLateOutliner::emitOMPReductionClause(const OMPReductionClause *Cl) {
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
  if (Cl->isImplicit()) {
    if (CurrentDirectiveKind == OMPD_target ||
        CurrentDirectiveKind == OMPD_task) {
      // Only insert this on topmost part of combined directives.
      for (const auto *E : Cl->varlists()) {
        if (const auto *DRE = dyn_cast<DeclRefExpr>(E)) {
          ImplicitMap.insert(std::make_pair(cast<VarDecl>(DRE->getDecl()),
                                            ICK_specified_firstprivate));
        }
      }
    }
    return;
  }
  auto *IPriv = Cl->private_copies().begin();
  for (auto *E : Cl->varlists()) {
    ClauseEmissionHelper CEH(*this, OMPC_firstprivate, "QUAL.OMP.FIRSTPRIVATE");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    const VarDecl *PVD = getExplicitVarDecl(E);
    assert(PVD && "expected VarDecl in firstprivate clause");
    addExplicit(PVD);
    bool IsPODType = E->getType().isPODType(CGF.getContext());
    bool IsCapturedExpr = isa<OMPCapturedExprDecl>(PVD);
    bool IsRef = !IsCapturedExpr && PVD->getType()->isReferenceType();
    if (!IsPODType)
      CSB.setNonPod();
    if (IsRef)
      CSB.setByRef();
    addArg(CSB.getString());
    addArg(E, IsRef);
    if (!IsPODType) {
      addArg(emitOpenMPCopyConstructor(*IPriv));
      addArg(emitOpenMPDestructor(E->getType()));
    }
    ++IPriv;
  }
}

void OpenMPLateOutliner::emitOMPCopyinClause(const OMPCopyinClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_copyin);
  addArg("QUAL.OMP.COPYIN");
  for (auto *E : Cl->varlists()) {
    if (!E->getType().isPODType(CGF.getContext()))
      CGF.CGM.ErrorUnsupported(E, "non-POD copyin variable");
    const VarDecl *PVD = getExplicitVarDecl(E);
    assert(PVD && "expected VarDecl in copyin clause");
    addExplicit(PVD);
    addArg(E);
  }
}

void OpenMPLateOutliner::emitOMPIfClause(const OMPIfClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_if);
  addArg("QUAL.OMP.IF");
  addArg(CGF.EmitScalarExpr(Cl->getCondition()));
}

void OpenMPLateOutliner::emitOMPNumThreadsClause(
    const OMPNumThreadsClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_num_threads);
  addArg("QUAL.OMP.NUM_THREADS");
  addArg(CGF.EmitScalarExpr(Cl->getNumThreads()));
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
  case OMPC_DEFAULT_none:
    addArg("QUAL.OMP.DEFAULT.NONE");
    break;
  case OMPC_DEFAULT_shared:
    addArg("QUAL.OMP.DEFAULT.SHARED");
    break;
  case OMPC_DEFAULT_unknown:
    llvm_unreachable("Unknown default clause");
  }
}

void OpenMPLateOutliner::emitOMPProcBindClause(const OMPProcBindClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_proc_bind);
  switch (Cl->getProcBindKind()) {
  case OMPC_PROC_BIND_master:
    addArg("QUAL.OMP.PROC_BIND.MASTER");
    break;
  case OMPC_PROC_BIND_close:
    addArg("QUAL.OMP.PROC_BIND.CLOSE");
    break;
  case OMPC_PROC_BIND_spread:
    addArg("QUAL.OMP.PROC_BIND.SPREAD");
    break;
  case OMPC_PROC_BIND_unknown:
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
  ClauseEmissionHelper CEH(*this, OMPC_collapse);
  addArg("QUAL.OMP.COLLAPSE");
  addArg(CGF.EmitScalarExpr(Cl->getNumForLoops()));
}

void OpenMPLateOutliner::emitOMPAlignedClause(const OMPAlignedClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_aligned);
  addArg("QUAL.OMP.ALIGNED");
  for (auto *E : Cl->varlists())
    addArg(E);
  addArg(Cl->getAlignment() ? CGF.EmitScalarExpr(Cl->getAlignment())
                            : CGF.Builder.getInt32(0));
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
  addArg(CGF.EmitScalarExpr(Cl->getCondition()));
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
  auto DepKind = Cl->getDependencyKind();

  // The depend clause is added to the implicit task only.
  if (isImplicitTask(OMPD_target))
    return;
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
    if (E->getType()->isSpecificPlaceholderType(BuiltinType::OMPArraySection))
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
  ClauseEmissionHelper CEH(*this, OMPC_is_device_ptr);
  addArg("QUAL.OMP.IS_DEVICE_PTR");
  for (auto *E : Cl->varlists()) {
    const VarDecl *PVD = getExplicitVarDecl(E);
    assert(PVD && "expected VarDecl in is_device_ptr clause");
    addExplicit(PVD);
    addArg(E);
  }
}

void OpenMPLateOutliner::emitOMPDefaultmapClause(
    const OMPDefaultmapClause *Cl) {

  if (Cl->getDefaultmapModifier() != OMPC_DEFAULTMAP_MODIFIER_tofrom ||
      Cl->getDefaultmapKind() != OMPC_DEFAULTMAP_scalar)
    llvm_unreachable("Unsupported defaultmap clause");

  ClauseEmissionHelper CEH(*this, OMPC_defaultmap);
  addArg("QUAL.OMP.DEFAULTMAP.TOFROM.SCALAR");
}

void OpenMPLateOutliner::emitOMPNowaitClause(const OMPNowaitClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_nowait);
  addArg("QUAL.OMP.NOWAIT");
}

void OpenMPLateOutliner::emitOMPUseDevicePtrClause(
    const OMPUseDevicePtrClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_use_device_ptr);
  addArg("QUAL.OMP.USE_DEVICE_PTR");
  for (auto *E : Cl->varlists())
    addArg(E);
}

void OpenMPLateOutliner::emitOMPToClause(const OMPToClause *Cl) {
  for (auto *E : Cl->varlists()) {
    ClauseEmissionHelper CEH(*this, OMPC_to, "QUAL.OMP.TO");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    if (isa<ArraySubscriptExpr>(E->IgnoreParenImpCasts()) ||
        E->getType()->isSpecificPlaceholderType(BuiltinType::OMPArraySection))
      CSB.setArrSect();
    addArg(CSB.getString());
    addArg(E);
  }
}

void OpenMPLateOutliner::emitOMPFromClause(const OMPFromClause *Cl) {
  for (auto *E : Cl->varlists()) {
    ClauseEmissionHelper CEH(*this, OMPC_from, "QUAL.OMP.FROM");
    ClauseStringBuilder &CSB = CEH.getBuilder();
    if (isa<ArraySubscriptExpr>(E->IgnoreParenImpCasts()) ||
        E->getType()->isSpecificPlaceholderType(BuiltinType::OMPArraySection))
      CSB.setArrSect();
    addArg(CSB.getString());
    addArg(E);
  }
}

void OpenMPLateOutliner::emitOMPNumTeamsClause(const OMPNumTeamsClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_num_teams);
  addArg("QUAL.OMP.NUM_TEAMS");
  if (llvm::Value *V = CGF.getMappedClause(Cl)) {
    addValueRef(V);
    addArg(V, /*Handled=*/true);
  } else {
    addArg(CGF.EmitScalarExpr(Cl->getNumTeams()));
  }
}

void OpenMPLateOutliner::emitOMPThreadLimitClause(
    const OMPThreadLimitClause *Cl) {
  ClauseEmissionHelper CEH(*this, OMPC_thread_limit);
  addArg("QUAL.OMP.THREAD_LIMIT");
  if (llvm::Value *V = CGF.getMappedClause(Cl)) {
    addValueRef(V);
    addArg(V, /*Handled=*/true);
  } else {
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
    const VarDecl *PVD = getExplicitVarDecl(E);
    assert(PVD && "expected VarDecl in copyprivate clause");
    addExplicit(PVD);
    if (!IsPODType)
      CSB.setNonPod();
    addArg(CSB.getString());
    addArg(E);
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

void OpenMPLateOutliner::buildMapQualifier(ClauseStringBuilder &CSB,
                                           const OMPMapClause *C) {
  CSB.add("QUAL.OMP.MAP.");
  for (unsigned I = 0; I < OMPMapClause::NumberOfModifiers; ++I) {
    if (C->getMapTypeModifier(I) == OMPC_MAP_MODIFIER_always) {
      CSB.add("ALWAYS.");
      break;
    } else if (C->getMapTypeModifier(I) == OMPC_MAP_MODIFIER_unknown)
      break;
    else
      llvm_unreachable("Unexpected map modifier");
  }
  switch (C->getMapType()) {
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
}

void OpenMPLateOutliner::emitOMPMapClause(const OMPMapClause *C) {
  for (const auto *E : C->varlists()) {
    // When there is a map-chain in the IR for the the map clause, the
    //  computations for various expressions used in the map-chain are to be
    //  emitted after the implicit task, and before the target directive
    if (const VarDecl *PVD = getExplicitVarDecl(E)) {
      if (isImplicitTask(OMPD_task)) {
        ImplicitMap.insert(std::make_pair(PVD, ICK_shared));
        continue;
      } else {
        addExplicit(PVD, /*IsMap=*/true);
      }
    }
    SmallVector<CGOpenMPRuntime::MapInfo, 4> Info;
    {
      // Generate map values and emit outside the current directive.
      ClauseEmissionHelper CEH(*this, OMPC_map, /*InitStr=*/"",
                               /*EmitClause=*/false);
      CGOpenMPRuntime::getLOMapInfo(Directive, CGF, C, E, Info);
    }
    if (Info.size() == 1 && Info[0].Base == Info[0].Pointer) {
      // This is the simple non-aggregate case.
      ClauseEmissionHelper CEH(*this, OMPC_map);
      ClauseStringBuilder &CSB = CEH.getBuilder();
      buildMapQualifier(CSB, C);
      addArg(CSB.getString());
      addArg(Info[0].Base);
      continue;
    }
    for (auto I = Info.begin(), E = Info.end(); I != E; ++I) {
      ClauseEmissionHelper CEH(*this, OMPC_map);
      ClauseStringBuilder &CSB = CEH.getBuilder();
      buildMapQualifier(CSB, C);
      CSB.add(I == Info.begin() ? ":AGGRHEAD" : ":AGGR");
      addArg(CSB.getString());
      addArg(I->Base);
      addArg(I->Pointer);
      addArg(I->Size);
    }
  }
}

void OpenMPLateOutliner::emitOMPReadClause(const OMPReadClause *) {}
void OpenMPLateOutliner::emitOMPWriteClause(const OMPWriteClause *) {}
void OpenMPLateOutliner::emitOMPUpdateClause(const OMPUpdateClause *) {}
void OpenMPLateOutliner::emitOMPCaptureClause(const OMPCaptureClause *) {}
void OpenMPLateOutliner::emitOMPSeqCstClause(const OMPSeqCstClause *) {}
void OpenMPLateOutliner::emitOMPThreadsClause(const OMPThreadsClause *) {}
void OpenMPLateOutliner::emitOMPSIMDClause(const OMPSIMDClause *) {}
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
void OpenMPLateOutliner::emitOMPAllocateClause(const OMPAllocateClause *) {}

void OpenMPLateOutliner::addFenceCalls(bool IsBegin) {
  switch (Directive.getDirectiveKind()) {
  case OMPD_atomic:
  case OMPD_critical:
  case OMPD_single:
  case OMPD_master:
    if (IsBegin)
      CGF.Builder.CreateFence(llvm::AtomicOrdering::Acquire);
    else
      CGF.Builder.CreateFence(llvm::AtomicOrdering::Release);
    break;
  case OMPD_barrier:
  case OMPD_taskwait:
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

OpenMPLateOutliner::OpenMPLateOutliner(CodeGenFunction &CGF,
                                       const OMPExecutableDirective &D,
                                       OpenMPDirectiveKind Kind)
    : CGF(CGF), C(CGF.CGM.getLLVMContext()), VSMH(CGF), Directive(D),
      CurrentDirectiveKind(Kind) {
  // Set an attribute indicating that the routine may have OpenMP directives
  // (represented with llvm intrinsics) in the LLVM IR
  CGF.CurFn->addFnAttr("may-have-openmp-directive", "true");

  if (CurrentDirectiveKind == OMPD_unknown)
    CurrentDirectiveKind = D.getDirectiveKind();

  RegionEntryDirective =
      CGF.CGM.getIntrinsic(llvm::Intrinsic::directive_region_entry);
  RegionExitDirective =
      CGF.CGM.getIntrinsic(llvm::Intrinsic::directive_region_exit);

  if (isOpenMPLoopDirective(CurrentDirectiveKind)) {
    auto *LoopDir = dyn_cast<OMPLoopDirective>(&D);
    for (auto *E : LoopDir->counters()) {
      auto *PVD = cast<VarDecl>(cast<DeclRefExpr>(E)->getDecl());
      if (CurrentDirectiveKind == OMPD_simd) {
        if (CGF.IsPrivateCounter(PVD))
          ImplicitMap.insert(std::make_pair(PVD, ICK_unknown));
        else
          ImplicitMap.insert(std::make_pair(PVD, ICK_lastprivate));
      }
      else
        ImplicitMap.insert(std::make_pair(PVD, ICK_private));
    }
    for (const auto *C : LoopDir->getClausesOfKind<OMPOrderedClause>()) {
      if (!C->getNumForLoops())
        continue;
      for (unsigned I = LoopDir->getCollapsedNumber(),
                    E = C->getLoopNumIterations().size();
           I < E; ++I) {
        const auto *DRE = cast<DeclRefExpr>(C->getLoopCounter(I));
        const auto *PVD = cast<VarDecl>(DRE->getDecl());
        if (isOpenMPSimdDirective(CurrentDirectiveKind))
          ImplicitMap.insert(std::make_pair(PVD, ICK_unknown));
        else
          ImplicitMap.insert(std::make_pair(PVD, ICK_private));
      }
    }
#if INTEL_CUSTOMIZATION
    if (CGF.useUncollapsedLoop(*LoopDir)) {
      auto UncollapsedIVs = LoopDir->uncollapsedIVs();
      auto UncollapsedLowerBounds = LoopDir->uncollapsedLowerBounds();
      auto UncollapsedUpperBounds = LoopDir->uncollapsedUpperBounds();
      for (unsigned I = 0, E = LoopDir->getCollapsedNumber(); I < E; ++I) {
        HandleImplicitVar(UncollapsedIVs[I], ICK_normalized_iv);
        HandleImplicitVar(UncollapsedUpperBounds[I], ICK_normalized_ub);
        if (isOpenMPWorksharingDirective(CurrentDirectiveKind) ||
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
        isOpenMPTaskLoopDirective(CurrentDirectiveKind) ||
        isOpenMPDistributeDirective(CurrentDirectiveKind)) {
      HandleImplicitVar(LoopDir->getLowerBoundVariable(), ICK_firstprivate);
    }
  }
}

OpenMPLateOutliner::~OpenMPLateOutliner() {
  addImplicitClauses();

  // Insert the start directives.
  auto EndIP = CGF.Builder.saveIP();
  setInsertPoint();

  for (auto &D : Directives) {
    D.CallEntry = CGF.Builder.CreateCall(RegionEntryDirective, {}, D.OpBundles);
    D.clear();
    // Place the end directive in place of the start.
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
void OpenMPLateOutliner::emitOMPTargetDirective(int OffloadEntryIndex) {
  startDirectiveIntrinsicSet("DIR.OMP.TARGET", "DIR.OMP.END.TARGET",
                             OMPD_target);

  // Add operand bundle for the offload entry index.
  ClauseEmissionHelper CEH(*this, OMPC_unknown);
  addArg("QUAL.OMP.OFFLOAD.ENTRY.IDX");
  addArg(CGF.Builder.getInt32(OffloadEntryIndex));
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
  if (CGF.requiresImplicitTask(Directive)) {
    bool NeedIf = Directive.hasClausesOfKind<OMPDependClause>();
    NeedIf = NeedIf && !Directive.hasClausesOfKind<OMPNowaitClause>();
    if (NeedIf) {
      ClauseEmissionHelper CEH(*this, OMPC_if);
      addArg("QUAL.OMP.IF");
      addArg(CGF.Builder.getInt32(0));
    }
    ClauseEmissionHelper CEH(*this, OMPC_unknown);
    addArg("QUAL.OMP.TARGET.TASK");
  }
}
void OpenMPLateOutliner::emitOMPTaskGroupDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.TASKGROUP", "DIR.OMP.END.TASKGROUP",
                             OMPD_taskgroup);
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
                             OMPD_distribute_simd);
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
  if (isImplicitTask(OMPD_target)) {
    return Kind == OMPC_depend ||
           !isAllowedClauseForDirective(CurrentDirectiveKind, Kind);
  }
  if (isImplicitTask(OMPD_task)) {
    return Kind != OMPC_map &&
           !isAllowedClauseForDirective(CurrentDirectiveKind, Kind);
  }
  return !isAllowedClauseForDirective(CurrentDirectiveKind, Kind);
}

#if INTEL_CUSTOMIZATION
void OpenMPLateOutliner::emitOMPTargetVariantDispatchDirective() {
  startDirectiveIntrinsicSet("DIR.OMP.TARGET.VARIANT.DISPATCH",
                             "DIR.OMP.END.TARGET.VARIANT.DISPATCH",
                             OMPD_target_variant_dispatch);
}
#endif // INTEL_CUSTOMIZATION

OpenMPLateOutliner &OpenMPLateOutliner::
operator<<(ArrayRef<OMPClause *> Clauses) {
  for (auto *C : Clauses) {
    OpenMPClauseKind ClauseKind = C->getClauseKind();
    if (shouldSkipExplicitClause(ClauseKind))
      continue;
    switch (ClauseKind) {
#define OPENMP_CLAUSE(Name, Class)                                             \
  case OMPC_##Name:                                                            \
    emit##Class(cast<Class>(C));                                               \
    break;
#include "clang/Basic/OpenMPKinds.def"
    case OMPC_match:
    case OMPC_device_type:
    case OMPC_uniform:
    case OMPC_threadprivate:
    case OMPC_unknown:
      llvm_unreachable("Clause not allowed");
    }
  }
  return *this;
}

// OpenMP spec 5.0, sec 2.19.7:
// If a list item appears in a reduction, lastprivate or linear clause on a
// combined target construct then it is treated as if it also appears in a
// map clause with a map-type of tofrom.
void OpenMPLateOutliner::emitCombinedTargetMapClauses() {
  // Add to the target of a combined target construct.
  if (CurrentDirectiveKind != OMPD_target ||
      Directive.getDirectiveKind() == OMPD_target)
    return;

  for (const auto *C : Directive.getClausesOfKind<OMPReductionClause>())
    AddMapToFromClauses(C);
  for (const auto *C : Directive.getClausesOfKind<OMPLastprivateClause>())
    AddMapToFromClauses(C);
  for (const auto *C : Directive.getClausesOfKind<OMPLinearClause>())
    AddMapToFromClauses(C);
}

bool OpenMPLateOutliner::isFirstDirectiveInSet(const OMPExecutableDirective &S,
                                               OpenMPDirectiveKind Kind) {
  if (Kind == OMPD_unknown)
    return true;

  OpenMPDirectiveKind DKind = S.getDirectiveKind();
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
  case OMPD_target:
  case OMPD_target_enter_data:
  case OMPD_target_exit_data:
  case OMPD_target_update:
    if (S.hasClausesOfKind<OMPDependClause>() ||
        S.hasClausesOfKind<OMPNowaitClause>())
      return Kind == OMPD_task;
    return Kind == OMPD_target;

  case OMPD_teams_distribute:
  case OMPD_teams_distribute_simd:
  case OMPD_teams_distribute_parallel_for:
  case OMPD_teams_distribute_parallel_for_simd:
    return Kind == OMPD_teams;

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
  case OMPD_parallel:
  case OMPD_for:
  case OMPD_parallel_for:
  case OMPD_parallel_sections:
  case OMPD_for_simd:
  case OMPD_parallel_for_simd:
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
  case OMPD_target_update:
  case OMPD_taskloop:
  case OMPD_taskloop_simd:
  case OMPD_master_taskloop:
  case OMPD_master_taskloop_simd:
  case OMPD_parallel_master_taskloop:
    return true;
  case OMPD_cancel:
  case OMPD_cancellation_point:
  case OMPD_ordered:
  case OMPD_threadprivate:
  case OMPD_allocate:
  case OMPD_section:
  case OMPD_single:
  case OMPD_master:
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
#if INTEL_CUSTOMIZATION
  case OMPD_target_variant_dispatch:
#endif // INTEL_CUSTOMIZATION
  case OMPD_declare_reduction:
  case OMPD_declare_mapper:
  case OMPD_requires:
    return false;
  case OMPD_unknown:
    llvm_unreachable("Unexpected directive.");
  }
  return true;
}

/// Emit the captured statement body.
void CGLateOutlineOpenMPRegionInfo::EmitBody(CodeGenFunction &CGF,
                                             const Stmt *S) {
  if (!CGF.HaveInsertPoint())
    return;
  CGF.EHStack.pushTerminate();

  auto *CS = (cast<CapturedStmt>(S))->getCapturedStmt();
  // The OMP structured-block (captured statement) is a declaration. Create a
  // new exception scope so we can handle any lingering cleanup exceptions
  // (due to ill-formed block) before popping the terminate exception.
  if (isa<DeclStmt>(CS)) {
    CodeGenFunction::RunCleanupsScope Scope(CGF);
    CGF.EmitStmt(CS);
  } else {
    CGF.EmitStmt(CS);
  }

  CGF.EHStack.popTerminate();
}

/// Return true if processing the part of an implicit task corresponding to K.
bool OpenMPLateOutliner::isImplicitTask(OpenMPDirectiveKind K) {
  if (!CGF.requiresImplicitTask(Directive))
     return false;
   return K == CurrentDirectiveKind;
}

/// Return true if Directive require implicit task to be generated.
bool CodeGenFunction::requiresImplicitTask(
    const OMPExecutableDirective &Directive) {
  if ((isOpenMPTargetExecutionDirective(Directive.getDirectiveKind()) ||
       Directive.getDirectiveKind() == OMPD_target_enter_data ||
       Directive.getDirectiveKind() == OMPD_target_exit_data ||
       Directive.getDirectiveKind() == OMPD_target_update) &&
      (Directive.hasClausesOfKind<OMPDependClause>() ||
       Directive.hasClausesOfKind<OMPNowaitClause>()))
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

static OpenMPDirectiveKind nextDirectiveKind(OpenMPDirectiveKind FullDirKind,
                                             OpenMPDirectiveKind CurrDirKind) {
  if (CurrDirKind == OMPD_task) {
    if (FullDirKind == OMPD_target_enter_data ||
        FullDirKind == OMPD_target_exit_data ||
        FullDirKind == OMPD_target_update)
      return FullDirKind;
    return OMPD_target;
  }

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
  default:
    llvm_unreachable("Unhandled combined directive.");
  }
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
  case OMPD_critical: {
    const auto &CD = cast<OMPCriticalDirective>(S);
    Outliner.emitOMPCriticalDirective(CD.getDirectiveName().getAsString());
    break;
  }
  case OMPD_ordered:
    Outliner.emitOMPOrderedDirective();
    break;
  case OMPD_target: {
#if INTEL_CUSTOMIZATION
    CGM.setHasTargetCode();
#endif //INTEL_CUSTOMIZATION

    // Get an index of the associated offload entry for this target directive.
    assert(CurFuncDecl && "No parent declaration for target region!");
    StringRef ParentName;
    // In case we have Ctors/Dtors we use the complete type variant to produce
    // the mangling of the device outlined kernel.
    if (const auto *D = dyn_cast<CXXConstructorDecl>(CurFuncDecl))
      ParentName = CGM.getMangledName(GlobalDecl(D, Ctor_Complete));
    else if (const auto *D = dyn_cast<CXXDestructorDecl>(CurFuncDecl))
      ParentName = CGM.getMangledName(GlobalDecl(D, Dtor_Complete));
    else
      ParentName =
          CGM.getMangledName(GlobalDecl(cast<FunctionDecl>(CurFuncDecl)));

    int Order = CGM.getOpenMPRuntime().registerTargetRegion(S, ParentName);
    assert(Order >= 0 && "No entry for the target region");

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

#if INTEL_CUSTOMIZATION
  case OMPD_target_variant_dispatch:
    Outliner.emitOMPTargetVariantDispatchDirective();
    break;
#endif // INTEL_CUSTOMIZATION

  // These directives are not yet implemented.
  case OMPD_allocate:
  case OMPD_requires:
    break;

  // These directives do not create region directives.
  case OMPD_declare_target:
  case OMPD_end_declare_target:
  case OMPD_declare_variant:
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
  case OMPD_teams_distribute:
  case OMPD_teams_distribute_simd:
  case OMPD_teams_distribute_parallel_for:
  case OMPD_teams_distribute_parallel_for_simd:
  case OMPD_master_taskloop:
  case OMPD_master_taskloop_simd:
  case OMPD_parallel_master_taskloop:
    llvm_unreachable("Combined directives not handled here");
  }
  Outliner << S.clauses();
  Outliner.emitCombinedTargetMapClauses();
  Outliner.insertMarker();
  if (S.hasAssociatedStmt() && S.getAssociatedStmt() != nullptr) {
    LateOutlineOpenMPRegionRAII Region(*this, Outliner, S);
    if (S.getDirectiveKind() != CurrentDirectiveKind) {
      // Unless we've reached the innermost directive, keep going.
      OpenMPDirectiveKind NextKind =
          nextDirectiveKind(S.getDirectiveKind(), CurrentDirectiveKind);
      switch (NextKind) {
        case OMPD_parallel:
        case OMPD_teams:
        case OMPD_target:
        case OMPD_target_enter_data:
        case OMPD_target_exit_data:
        case OMPD_target_update:
          return EmitLateOutlineOMPDirective(S, NextKind);
        case OMPD_parallel_for:
        case OMPD_parallel_for_simd:
        case OMPD_simd:
        case OMPD_distribute:
        case OMPD_distribute_simd:
        case OMPD_distribute_parallel_for:
        case OMPD_distribute_parallel_for_simd:
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
      CurFn->addFnAttr("contains-openmp-target", "true");
    CodeGenModule::InTargetRegionRAII ITR(CGM, IsDeviceTarget);
    const Stmt *CapturedStmt = S.getInnermostCapturedStmt();
    CapturedStmtInfo->EmitBody(*this, CapturedStmt);
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
  for (const auto *Ref : RemapVars) {
    if (auto *DRE = dyn_cast<DeclRefExpr>(Ref->IgnoreParenImpCasts())) {
      if (auto *VD = dyn_cast<VarDecl>(DRE->getDecl())) {
        if (isa<OMPCapturedExprDecl>(VD)) {
          PrivScope.addPrivateNoTemps(VD, [this, VD]() -> Address {
            return EmitLValue(VD->getAnyInitializer()).getAddress();
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
    CurFn->addFnAttr("contains-openmp-target", "true");
  CodeGenModule::InTargetRegionRAII ITR(CGM, IsDeviceTarget);
  emitLateOutlineDirective(*this, CodeGen);
}
#endif // INTEL_COLLAB
