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

  void OpenMPCodeOutliner::addArg(llvm::Value *Val) { Args.push_back(Val); }
  void OpenMPCodeOutliner::addArg(StringRef Str) {
    Args.push_back(llvm::MetadataAsValue::get(C, llvm::MDString::get(C, Str)));
  }
  void OpenMPCodeOutliner::addArg(const Expr *E) {
    if (E->getType()->isSpecificPlaceholderType(BuiltinType::OMPArraySection)) {
      ArraySectionTy AS;
      Address Base = emitOMPArraySectionExpr(
          cast<OMPArraySectionExpr>(E->IgnoreParenImpCasts()), AS);
      addArg("QUAL.OPND.ARRSECT");
      addArg(Base.getPointer());
      addArg(llvm::ConstantInt::get(CGF.SizeTy, AS.size()));
      // If VLASize of the first element is not nullptr, we have sizes for all
      // dimensions of variably modified type.
      if (AS.begin()->VLASize) {
        addArg("QUAL.OPND.ARRSIZE");
        for (auto &V : AS) {
          assert(V.VLASize);
          Args.push_back(V.VLASize);
        }
      }
      for (auto &V : AS) {
        assert(V.LowerBound);
        Args.push_back(V.LowerBound);
        assert(V.Length);
        Args.push_back(V.Length);
        assert(V.Stride);
        Args.push_back(V.Stride);
      }
      return;
    }
    assert(E->isGLValue());
    addArg(CGF.EmitLValue(E).getPointer());
  }

  void OpenMPCodeOutliner::emitDirective() {
    llvm::CallInst *C = CGF.EmitRuntimeCall(IntelDirective, Args);
    if (!DirectiveInst)
      DirectiveInst = C;
    Args.clear();
  }
  void OpenMPCodeOutliner::emitSimpleClause() {
    assert(Args.size() == 1);
    CGF.EmitRuntimeCall(IntelSimpleClause, Args);
    Args.clear();
  }
  void OpenMPCodeOutliner::emitOpndClause() {
    assert(Args.size() == 2);
    llvm::Type *Types[] = {Args[1]->getType()};
    llvm::Function *IntelOpndClause =
        CGF.CGM.getIntrinsic(llvm::Intrinsic::intel_directive_qual_opnd, Types);
    CGF.EmitRuntimeCall(IntelOpndClause, Args);
    Args.clear();
  }
  void OpenMPCodeOutliner::emitListClause() {
    CGF.EmitRuntimeCall(IntelListClause, Args);
    Args.clear();
  }

  void OpenMPCodeOutliner::emitImplicit(Expr *E, OpenMPClauseKind K) {
    auto SavedIP = CGF.Builder.saveIP();
    CGF.Builder.SetInsertPoint(DirectiveInst->getNextNode());
    switch (K) {
    case OMPC_private:
      addArg("QUAL.OMP.PRIVATE"); break;
    case OMPC_shared: 
      addArg("QUAL.OMP.SHARED"); break;
    default:
      llvm_unreachable("Clause not allowed");
    }
    addArg(E);
    emitListClause();
    CGF.Builder.restoreIP(SavedIP);
  }

  void OpenMPCodeOutliner::emitImplicit(const VarDecl *VD, OpenMPClauseKind K) {
    DeclRefExpr DRE(const_cast<VarDecl *>(VD),
                    /*RefersToEnclosingVariableOrCapture=*/false,
                    VD->getType().getNonReferenceType(), VK_LValue,
                    SourceLocation());
    emitImplicit(&DRE, K);
  }

  void OpenMPCodeOutliner::addImplicitClauses() {
    auto DKind = Directive.getDirectiveKind();
    if (DKind != OMPD_simd && !isOpenMPParallelDirective(DKind))
      return;

    // We need to process some DeclRefExprs without havining them affect the
    // Def/Ref lists so save and null the CapturedStmtInfo.
    auto savedCSI = CGF.CapturedStmtInfo;
    CGF.CapturedStmtInfo = nullptr;

    auto End = ExplicitRefs.end();
    for (const auto *I : VarRefs) {
      if (ExplicitRefs.find(I) != End) continue;
      if (VarDefs.find(I) != VarDefs.end()) {
        // Defined here = private
        emitImplicit(I, OMPC_private);
      } else if (DKind != OMPD_simd) {
        // Referenced but not definted = shared
        emitImplicit(I, OMPC_shared);
      }
    }

    // Restore the CSI
    CGF.CapturedStmtInfo = savedCSI;
  }

  void OpenMPCodeOutliner::addRefsToOuter() {
    if (CGF.CapturedStmtInfo) {
      for (const auto *I : VarDefs)
        CGF.CapturedStmtInfo->recordVariableDefinition(I);
      for (const auto *I : VarRefs)
        CGF.CapturedStmtInfo->recordVariableReference(I);
      for (const auto *I : ExplicitRefs)
        CGF.CapturedStmtInfo->recordVariableReference(I);
    }
  }

  void OpenMPCodeOutliner::emitOMPSharedClause(const OMPSharedClause *Cl) {
    addArg("QUAL.OMP.SHARED");
    for (auto *E : Cl->varlists())
      addArg(E);
    emitListClause();
  }
  void OpenMPCodeOutliner::emitOMPPrivateClause(const OMPPrivateClause *Cl) {
    addArg("QUAL.OMP.PRIVATE");
    auto IPriv = Cl->private_copies().begin();
    for (auto *E : Cl->varlists()) {
      auto *PVD = cast<VarDecl>(cast<DeclRefExpr>(E)->getDecl());
      addExplicit(PVD);
      auto *Private = cast<VarDecl>(cast<DeclRefExpr>(*IPriv)->getDecl());
      auto *Init = Private->getInit();
      if (Init || Private->getType().isDestructedType())
        addArg("QUAL.OPND.NONPOD");
      addArg(E);
      if (Init || Private->getType().isDestructedType()) {
        addArg(emitIntelOpenMPDefaultConstructor(*IPriv));
        addArg(emitIntelOpenMPDestructor(Private->getType()));
      }
      ++IPriv;
    }
    emitListClause();
  }
  void OpenMPCodeOutliner::emitOMPLastprivateClause(
                                     const OMPLastprivateClause *Cl) {
    addArg("QUAL.OMP.LASTPRIVATE");
    auto IPriv = Cl->private_copies().begin();
    auto ISrcExpr = Cl->source_exprs().begin();
    auto IDestExpr = Cl->destination_exprs().begin();
    auto IAssignOp = Cl->assignment_ops().begin();
    for (auto *E : Cl->varlists()) {
      bool IsPODType = E->getType().isPODType(CGF.getContext());
      if (!IsPODType)
        addArg("QUAL.OPND.NONPOD");
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
    }
    emitListClause();
  }
  void OpenMPCodeOutliner::emitOMPLinearClause(const OMPLinearClause *Cl) {
    StringRef Linear;
    switch (Cl->getModifier()) {
    case OMPC_LINEAR_ref:
      Linear = "QUAL.OMP.LINEAR.REF";
      break;
    case OMPC_LINEAR_val:
      Linear = "QUAL.OMP.LINEAR.VAL";
      break;
    case OMPC_LINEAR_uval:
      Linear = "QUAL.OMP.LINEAR.UVAL";
      break;
    case OMPC_LINEAR_unknown:
      llvm_unreachable("Wrong linear modifier");
    }
    addArg(Linear);
    for (auto *E : Cl->varlists())
      addArg(E);
    addArg(Cl->getStep() ? CGF.EmitScalarExpr(Cl->getStep())
                         : CGF.Builder.getInt32(1));
    emitListClause();
  }
  void OpenMPCodeOutliner::emitOMPReductionClause(
                                      const OMPReductionClause *Cl) {
    StringRef Op;
    OverloadedOperatorKind OOK =
        Cl->getNameInfo().getName().getCXXOverloadedOperator();
    auto I = Cl->reduction_ops().begin();
    for (auto *E : Cl->varlists()) {
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
        }
        break;
      }
      addArg(Op);
      addArg(E);
      emitListClause();
      ++I;
    }
  }

  void OpenMPCodeOutliner::emitOMPOrderedClause(const OMPOrderedClause *C) {
    addArg("QUAL.OMP.ORDERED");
    if (auto *E = C->getNumForLoops())
      addArg(CGF.EmitScalarExpr(E));
    else
      addArg(CGF.Builder.getInt32(1));
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
    switch (C->getScheduleKind()) {
    case OMPC_SCHEDULE_static:
      addArg("QUAL.OMP.SCHEDULE.STATIC");
      break;
    case OMPC_SCHEDULE_dynamic:
      DefaultChunkSize = 1;
      addArg("QUAL.OMP.SCHEDULE.DYNAMIC");
      break;
    case OMPC_SCHEDULE_guided:
      DefaultChunkSize = 1;
      addArg("QUAL.OMP.SCHEDULE.GUIDED");
      break;
    case OMPC_SCHEDULE_auto:
      addArg("QUAL.OMP.SCHEDULE.AUTO");
      break;
    case OMPC_SCHEDULE_runtime:
      addArg("QUAL.OMP.SCHEDULE.RUNTIME");
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
    if (Modifiers.empty())
      Modifiers = "MODIFIERNONE";
    addArg(Modifiers);
    if (auto *E = C->getChunkSize())
      addArg(CGF.EmitScalarExpr(E));
    else
      addArg(CGF.Builder.getInt32(DefaultChunkSize));
    emitListClause();
  }

  void OpenMPCodeOutliner::emitOMPFirstprivateClause(
                                  const OMPFirstprivateClause *Cl) {
    addArg("QUAL.OMP.FIRSTPRIVATE");
    auto *IPriv = Cl->private_copies().begin();
    for (auto *E : Cl->varlists()) {
      bool IsPODType = E->getType().isPODType(CGF.getContext());
      if (!IsPODType)
        addArg("QUAL.OPND.NONPOD");
      addArg(E);
      if (!IsPODType) {
        addArg(emitIntelOpenMPCopyConstructor(*IPriv));
        addArg(emitIntelOpenMPDestructor(E->getType()));
      }
      ++IPriv;
    }
    emitListClause();
  }

  void OpenMPCodeOutliner::emitOMPCopyinClause(const OMPCopyinClause *Cl) {
    addArg("QUAL.OMP.COPYIN");
    for (auto *E : Cl->varlists()) {
      if (!E->getType().isPODType(CGF.getContext()))
        CGF.CGM.ErrorUnsupported(E, "non-POD copyin variable");
      auto *PVD = cast<VarDecl>(cast<DeclRefExpr>(E)->getDecl());
      addExplicit(PVD);
      addArg(E);
    }
    emitListClause();
  }

  void OpenMPCodeOutliner::emitOMPIfClause(const OMPIfClause *Cl) {
    addArg("QUAL.OMP.IF");
    addArg(CGF.EmitScalarExpr(Cl->getCondition()));
    emitOpndClause();
  }

  void OpenMPCodeOutliner::emitOMPNumThreadsClause(
                                    const OMPNumThreadsClause *Cl) {
    addArg("QUAL.OMP.NUM_THREADS");
    addArg(CGF.EmitScalarExpr(Cl->getNumThreads()));
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
    addArg(CGF.EmitScalarExpr(Cl->getSafelen()));
    emitOpndClause();
  }

  void OpenMPCodeOutliner::emitOMPSimdlenClause(const OMPSimdlenClause *Cl) {
    addArg("QUAL.OMP.SIMDLEN");
    addArg(CGF.EmitScalarExpr(Cl->getSimdlen()));
    emitOpndClause();
  }

  void OpenMPCodeOutliner::emitOMPCollapseClause(const OMPCollapseClause *Cl) {
    addArg("QUAL.OMP.COLLAPSE");
    addArg(CGF.EmitScalarExpr(Cl->getNumForLoops()));
    emitOpndClause();
  }

  void OpenMPCodeOutliner::emitOMPAlignedClause(const OMPAlignedClause *Cl) {
    addArg("QUAL.OMP.ALIGNED");
    for (auto *E : Cl->varlists())
      addArg(E);
    addArg(Cl->getAlignment() ? CGF.EmitScalarExpr(Cl->getAlignment())
                              : CGF.Builder.getInt32(0));
    emitListClause();
  }

  void OpenMPCodeOutliner::emitOMPFinalClause(const OMPFinalClause *) {}
  void OpenMPCodeOutliner::emitOMPCopyprivateClause(
                                        const OMPCopyprivateClause *) {}
  void OpenMPCodeOutliner::emitOMPNowaitClause(const OMPNowaitClause *) {}
  void OpenMPCodeOutliner::emitOMPUntiedClause(const OMPUntiedClause *) {}
  void OpenMPCodeOutliner::emitOMPMergeableClause(const OMPMergeableClause *) {}
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
  void OpenMPCodeOutliner::emitOMPPriorityClause(const OMPPriorityClause *) {}
  void OpenMPCodeOutliner::emitOMPGrainsizeClause(const OMPGrainsizeClause *) {}
  void OpenMPCodeOutliner::emitOMPNogroupClause(const OMPNogroupClause *) {}
  void OpenMPCodeOutliner::emitOMPNumTasksClause(const OMPNumTasksClause *) {}
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
      : CGF(CGF), C(CGF.CGM.getLLVMContext()), DirectiveInst(nullptr),
        Directive(D) {
    IntelDirective = CGF.CGM.getIntrinsic(llvm::Intrinsic::intel_directive);
    IntelSimpleClause =
        CGF.CGM.getIntrinsic(llvm::Intrinsic::intel_directive_qual);
    IntelListClause =
        CGF.CGM.getIntrinsic(llvm::Intrinsic::intel_directive_qual_opndlist);
  }
  OpenMPCodeOutliner::~OpenMPCodeOutliner() {
    addImplicitClauses();
    if (!End.empty()) {
      addArg(End);
      emitDirective();
      addArg("DIR.QUAL.LIST.END");
      emitDirective();
    }
    addRefsToOuter();
  }
  void OpenMPCodeOutliner::emitOMPParallelDirective() {
    End = "DIR.OMP.END.PARALLEL";
    addArg("DIR.OMP.PARALLEL");
    emitDirective();
  }
  void OpenMPCodeOutliner::emitOMPParallelForDirective() {
    End = "DIR.OMP.END.PARALLEL.LOOP";
    addArg("DIR.OMP.PARALLEL.LOOP");
    emitDirective();
  }
  void OpenMPCodeOutliner::emitOMPSIMDDirective() {
    End = "DIR.OMP.END.SIMD";
    addArg("DIR.OMP.SIMD");
    emitDirective();
  }
  void OpenMPCodeOutliner::emitOMPAtomicDirective(OMPAtomicClause ClauseKind) {
    End = "DIR.OMP.END.ATOMIC";
    addArg("DIR.OMP.ATOMIC");
    emitDirective();
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
    End = "DIR.OMP.END.SINGLE";
    addArg("DIR.OMP.SINGLE");
    emitDirective();
  }
  void OpenMPCodeOutliner::emitOMPMasterDirective() {
    End = "DIR.OMP.END.MASTER";
    addArg("DIR.OMP.MASTER");
    emitDirective();
  }
  void OpenMPCodeOutliner::emitOMPCriticalDirective() {
    End = "DIR.OMP.END.CRITICAL";
    addArg("DIR.OMP.CRITICAL");
    emitDirective();
  }
  void OpenMPCodeOutliner::emitOMPOrderedDirective() {
    End = "DIR.OMP.END.ORDERED";
    addArg("DIR.OMP.ORDERED");
    emitDirective();
  }
  void OpenMPCodeOutliner::emitOMPTargetDirective() {
    End = "DIR.OMP.END.TARGET";
    addArg("DIR.OMP.TARGET");
    emitDirective();
  }
  OpenMPCodeOutliner &OpenMPCodeOutliner::operator<<(
                                         ArrayRef<OMPClause *> Clauses) {
    for (auto *C : Clauses) {
      switch (C->getClauseKind()) {
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
    addArg("DIR.QUAL.LIST.END");
    emitDirective();
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

void CodeGenFunction::EmitIntelOpenMPDirective(
    const OMPExecutableDirective &S) {
  OpenMPCodeOutliner Outliner(*this, S);
  switch (S.getDirectiveKind()) {
  case OMPD_parallel:
    Outliner.emitOMPParallelDirective();
    break;
  case OMPD_parallel_for:
    Outliner.emitOMPParallelForDirective();
    break;
  case OMPD_simd:
    Outliner.emitOMPSIMDDirective();
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
  case OMPD_for:
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
  case OMPD_teams_distribute_parallel_for:
  case OMPD_teams_distribute_parallel_for_simd:
  case OMPD_cancel:
  case OMPD_target_data:
  case OMPD_parallel_for_simd:
  case OMPD_parallel_sections:
  case OMPD_for_simd:
  case OMPD_cancellation_point:
  case OMPD_taskloop:
  case OMPD_taskloop_simd:
  case OMPD_distribute:
  case OMPD_target_enter_data:
  case OMPD_target_exit_data:
  case OMPD_target_parallel:
  case OMPD_target_parallel_for:
  case OMPD_target_parallel_for_simd:
  case OMPD_target_simd:
  case OMPD_target_update:
  case OMPD_target_teams:
  case OMPD_target_teams_distribute:
  case OMPD_target_teams_distribute_simd:
  case OMPD_target_teams_distribute_parallel_for:
  case OMPD_target_teams_distribute_parallel_for_simd:
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
  }
  Outliner << S.clauses();
  if (S.hasAssociatedStmt()) {
    InlinedOpenMPRegionRAII Region(*this, Outliner, S);
    CapturedStmtInfo->EmitBody(*this, S.getAssociatedStmt());
  }
}
#endif // INTEL_SPECIFIC_OPENMP
