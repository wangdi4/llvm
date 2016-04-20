//===--- CGIntelStmtOpenMP.cpp - Emit Intel Code from OpenMP Directives ---===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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
#include "../CGCXXABI.h"
#include "../CodeGenFunction.h"
#include "../CodeGenModule.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/StmtOpenMP.h"
using namespace clang;
using namespace CodeGen;

static LValue emitLoadOfPointerLValue(CodeGenFunction &CGF, Address PtrAddr,
                                      QualType Ty) {
  AlignmentSource Source;
  CharUnits Align = CGF.getNaturalPointeeTypeAlignment(Ty, &Source);
  return CGF.MakeAddrLValue(Address(CGF.Builder.CreateLoad(PtrAddr), Align),
                            Ty->getPointeeType(), Source);
}

static llvm::Value *emitIntelOpenMPDefaultConstructor(CodeGenModule &CGM,
                                                      const VarDecl *Private) {
  QualType Ty = Private->getType();

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
  CodeGenFunction CGF(CGM);
  FunctionArgList Args;
  ImplicitParamDecl Dst(CGM.getContext(), /*DC=*/nullptr, SourceLocation(),
                        /*Id=*/nullptr, PtrTy);
  Args.push_back(&Dst);

  auto &FI = CGM.getTypes().arrangeBuiltinFunctionDeclaration(PtrTy, Args);
  auto FTy = CGM.getTypes().GetFunctionType(FI);
  auto *Fn = CGM.CreateGlobalInitOrDestructFunction(FTy, OutName, FI);
  CGF.StartFunction(GlobalDecl(), PtrTy, Fn, FI, Args, SourceLocation());
  auto *Init = Private->getInit();
  if (Init && !CGF.isTrivialInitializer(Init)) {
    CodeGenFunction::RunCleanupsScope Scope(CGF);
    LValue ArgLVal =
        emitLoadOfPointerLValue(CGF, CGF.GetAddrOfLocalVar(&Dst), PtrTy);
    CGF.EmitAnyExprToMem(Init, ArgLVal.getAddress(), Ty.getQualifiers(),
                         /*IsInitializer=*/true);
    CGF.Builder.CreateStore(ArgLVal.getPointer(), CGF.ReturnValue);
  }
  CGF.FinishFunction();
  return Fn;
}

static llvm::Value *emitIntelOpenMPDestructor(CodeGenModule &CGM,
                                              const VarDecl *Private) {
  QualType Ty = Private->getType();

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
  CodeGenFunction CGF(CGM);
  FunctionArgList Args;
  ImplicitParamDecl Dst(CGM.getContext(), /*DC=*/nullptr, SourceLocation(),
                        /*Id=*/nullptr, PtrTy);
  Args.push_back(&Dst);

  auto &FI = CGM.getTypes().arrangeBuiltinFunctionDeclaration(
      CGM.getContext().VoidTy, Args);
  auto FTy = CGM.getTypes().GetFunctionType(FI);
  auto *Fn = CGM.CreateGlobalInitOrDestructFunction(FTy, OutName, FI);
  CGF.StartFunction(GlobalDecl(), CGM.getContext().VoidTy, Fn, FI, Args,
                    SourceLocation());
  if (Ty.isDestructedType() != QualType::DK_none) {
    CodeGenFunction::RunCleanupsScope Scope(CGF);
    LValue ArgLVal =
        emitLoadOfPointerLValue(CGF, CGF.GetAddrOfLocalVar(&Dst), PtrTy);
    CGF.emitDestroy(ArgLVal.getAddress(), Ty,
                    CGF.getDestroyer(Ty.isDestructedType()),
                    CGF.needsEHCleanup(Ty.isDestructedType()));
  }
  CGF.FinishFunction();
  return Fn;
}

namespace {
enum OMPAtomicClause {
  OMP_read,
  OMP_write,
  OMP_update,
  OMP_capture,
  OMP_read_seq_cst,
  OMP_write_seq_cst,
  OMP_update_seq_cst,
  OMP_capture_seq_cst,
};
/// Class used for emission of Intel-specific intrinsics for OpenMP code.
class OpenMPCodeOutliner {
  CodeGenFunction &CGF;
  StringRef End;
  bool WithListEnd = true;
  llvm::Function *IntelDirective = nullptr;
  llvm::Function *IntelSimpleClause = nullptr;
  llvm::Function *IntelOpndClause = nullptr;
  llvm::Function *IntelListClause = nullptr;
  llvm::LLVMContext &C;
  llvm::SmallVector<llvm::Value *, 16> Args;
  void addArg(StringRef Str) {
    Args.push_back(llvm::MetadataAsValue::get(C, llvm::MDString::get(C, Str)));
  }
  void addArg(llvm::Value *Val) { Args.push_back(Val); }
  void emitDirective() {
    CGF.EmitRuntimeCall(IntelDirective, Args);
    Args.clear();
  }
  void emitSimpleClause() {
    CGF.EmitRuntimeCall(IntelSimpleClause, Args);
    Args.clear();
  }
  void emitOpndClause() {
    assert(Args.size() == 2);
    llvm::Type *Types[] = {Args[0]->getType(), Args[1]->getType()};
    IntelOpndClause =
        CGF.CGM.getIntrinsic(llvm::Intrinsic::intel_directive_qual_opnd, Types);
    CGF.EmitRuntimeCall(IntelOpndClause, Args);
    Args.clear();
  }
  void emitListClause() {
    CGF.EmitRuntimeCall(IntelListClause, Args);
    Args.clear();
  }

  void emitOMPSharedClause(const OMPSharedClause *Cl) {
    addArg("QUAL.OMP.SHARED");
    for (auto *E : Cl->varlists()) {
      addArg(CGF.EmitLValue(E).getPointer());
    }
    emitListClause();
  }
  void emitOMPPrivateClause(const OMPPrivateClause *Cl) {
    addArg("QUAL.OMP.PRIVATE");
    auto IPriv = Cl->private_copies().begin();
    for (auto *E : Cl->varlists()) {
      auto *Private = cast<VarDecl>(cast<DeclRefExpr>(*IPriv)->getDecl());
      auto *Init = Private->getInit();
      if (Init || Private->getType().isDestructedType())
        addArg("QUAL.OPND.NONPOD");
      addArg(CGF.EmitLValue(E).getPointer());
      if (Init || Private->getType().isDestructedType()) {
        addArg(emitIntelOpenMPDefaultConstructor(CGF.CGM, Private));
        addArg(emitIntelOpenMPDestructor(CGF.CGM, Private));
      }
      ++IPriv;
    }
    emitListClause();
  }
  void emitOMPLinearClause(const OMPLinearClause *Cl) {
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
      addArg(CGF.EmitLValue(E).getPointer());
    addArg(Cl->getStep() ? CGF.EmitScalarExpr(Cl->getStep())
                         : CGF.Builder.getInt32(1));
    emitListClause();
  }
  void emitOMPReductionClause(const OMPReductionClause *Cl) {
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
      addArg(CGF.EmitLValue(E).getPointer());
      emitListClause();
      ++I;
    }
  }
  void emitOMPOrderedClause(const OMPOrderedClause *C) {
    addArg("QUAL.OMP.ORDERED");
    if (auto *E = C->getNumForLoops()) {
      addArg(CGF.EmitScalarExpr(E));
      emitOpndClause();
    } else
      emitSimpleClause();
  }
  void emitOMPIfClause(const OMPIfClause *) {}
  void emitOMPFinalClause(const OMPFinalClause *) {}
  void emitOMPNumThreadsClause(const OMPNumThreadsClause *) {}
  void emitOMPSafelenClause(const OMPSafelenClause *) {}
  void emitOMPSimdlenClause(const OMPSimdlenClause *) {}
  void emitOMPCollapseClause(const OMPCollapseClause *) {}
  void emitOMPDefaultClause(const OMPDefaultClause *) {}
  void emitOMPFirstprivateClause(const OMPFirstprivateClause *) {}
  void emitOMPLastprivateClause(const OMPLastprivateClause *) {}
  void emitOMPAlignedClause(const OMPAlignedClause *) {}
  void emitOMPCopyinClause(const OMPCopyinClause *) {}
  void emitOMPCopyprivateClause(const OMPCopyprivateClause *) {}
  void emitOMPProcBindClause(const OMPProcBindClause *) {}
  void emitOMPScheduleClause(const OMPScheduleClause *) {}
  void emitOMPNowaitClause(const OMPNowaitClause *) {}
  void emitOMPUntiedClause(const OMPUntiedClause *) {}
  void emitOMPMergeableClause(const OMPMergeableClause *) {}
  void emitOMPFlushClause(const OMPFlushClause *) {}
  void emitOMPReadClause(const OMPReadClause *) {}
  void emitOMPWriteClause(const OMPWriteClause *) {}
  void emitOMPUpdateClause(const OMPUpdateClause *) {}
  void emitOMPCaptureClause(const OMPCaptureClause *) {}
  void emitOMPSeqCstClause(const OMPSeqCstClause *) {}
  void emitOMPDependClause(const OMPDependClause *) {}
  void emitOMPDeviceClause(const OMPDeviceClause *) {}
  void emitOMPThreadsClause(const OMPThreadsClause *) {}
  void emitOMPSIMDClause(const OMPSIMDClause *) {}
  void emitOMPMapClause(const OMPMapClause *) {}
  void emitOMPNumTeamsClause(const OMPNumTeamsClause *) {}
  void emitOMPThreadLimitClause(const OMPThreadLimitClause *) {}
  void emitOMPPriorityClause(const OMPPriorityClause *) {}
  void emitOMPGrainsizeClause(const OMPGrainsizeClause *) {}
  void emitOMPNogroupClause(const OMPNogroupClause *) {}
  void emitOMPNumTasksClause(const OMPNumTasksClause *) {}
  void emitOMPHintClause(const OMPHintClause *) {}
  void emitOMPDistScheduleClause(const OMPDistScheduleClause *) {}
  void emitOMPDefaultmapClause(const OMPDefaultmapClause *) {}

public:
  OpenMPCodeOutliner(CodeGenFunction &CGF)
      : CGF(CGF), C(CGF.CGM.getLLVMContext()) {
    IntelDirective = CGF.CGM.getIntrinsic(llvm::Intrinsic::intel_directive);
    IntelSimpleClause =
        CGF.CGM.getIntrinsic(llvm::Intrinsic::intel_directive_qual);
    llvm::Type *Args[] = {CGF.VoidTy, CGF.VoidPtrTy, CGF.VoidPtrTy};
    IntelOpndClause =
        CGF.CGM.getIntrinsic(llvm::Intrinsic::intel_directive_qual_opnd, Args);
    IntelListClause =
        CGF.CGM.getIntrinsic(llvm::Intrinsic::intel_directive_qual_opndlist);
  }
  ~OpenMPCodeOutliner() {
    if (!End.empty()) {
      addArg(End);
      emitDirective();
      if (WithListEnd) {
        addArg("DIR.QUAL.LIST.END");
        emitDirective();
      }
    }
  }
  void emitOMPParallelDirective() {
    End = "DIR.OMP.END.PARALLEL";
    addArg("DIR.OMP.PARALLEL");
    emitDirective();
  }
  void emitOMPParallelForDirective() {
    End = "DIR.OMP.END.PARALLEL.LOOP";
    addArg("DIR.OMP.PARALLEL.LOOP");
    emitDirective();
  }
  void emitOMPSIMDDirective() {
    End = "DIR.OMP.END.SIMD";
    addArg("DIR.OMP.SIMD");
    emitDirective();
  }
  void emitOMPAtomicDirective(OMPAtomicClause ClauseKind) {
    WithListEnd = false;
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
  void emitOMPSingleDirective() {
    WithListEnd = false;
    End = "DIR.OMP.END.SINGLE";
    addArg("DIR.OMP.SINGLE");
    emitDirective();
  }
  void emitOMPMasterDirective() {
    WithListEnd = false;
    End = "DIR.OMP.END.MASTER";
    addArg("DIR.OMP.MASTER");
    emitDirective();
  }
  void emitOMPCriticalDirective() {
    WithListEnd = false;
    End = "DIR.OMP.END.CRITICAL";
    addArg("DIR.OMP.CRITICAL");
    emitDirective();
  }
  void emitOMPOrderedDirective() {
    WithListEnd = false;
    End = "DIR.OMP.END.ORDERED";
    addArg("DIR.OMP.ORDERED");
    emitDirective();
  }
  OpenMPCodeOutliner &operator<<(ArrayRef<OMPClause *> Clauses) {
    for (auto *C : Clauses) {
      switch (C->getClauseKind()) {
#define OPENMP_CLAUSE(Name, Class)                                             \
  case OMPC_##Name:                                                            \
    emit##Class(cast<Class>(C));                                               \
    break;
#include "clang/Basic/OpenMPKinds.def"
      case OMPC_threadprivate:
      case OMPC_unknown:
        llvm_unreachable("Clause not allowed");
      }
    }
    addArg("DIR.QUAL.LIST.END");
    emitDirective();
    return *this;
  }
};

/// Base class for handling code generation inside OpenMP regions.
class CGOpenMPRegionInfo : public CodeGenFunction::CGCapturedStmtInfo {
public:
  CGOpenMPRegionInfo(CodeGenFunction::CGCapturedStmtInfo *OldCSI,
                     const OMPExecutableDirective &D)
      : CGCapturedStmtInfo(*cast<CapturedStmt>(D.getAssociatedStmt()),
                           CR_OpenMP),
        OldCSI(OldCSI), D(D) {}

  /// \brief Emit the captured statement body.
  void EmitBody(CodeGenFunction &CGF, const Stmt *S) override {
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
  llvm::Value *getContextValue() const override {
    if (OldCSI)
      return OldCSI->getContextValue();
    llvm_unreachable("No context value for inlined OpenMP region");
  }
  void setContextValue(llvm::Value *V) override {
    if (OldCSI) {
      OldCSI->setContextValue(V);
      return;
    }
    llvm_unreachable("No context value for inlined OpenMP region");
  }
  /// \brief Lookup the captured field decl for a variable.
  const FieldDecl *lookup(const VarDecl *VD) const override {
    if (OldCSI)
      return OldCSI->lookup(VD);
    // If there is no outer outlined region,no need to lookup in a list of
    // captured variables, we can use the original one.
    return nullptr;
  }
  FieldDecl *getThisFieldDecl() const override {
    if (OldCSI)
      return OldCSI->getThisFieldDecl();
    return nullptr;
  }

  CodeGenFunction::CGCapturedStmtInfo *getOldCSI() const { return OldCSI; }

private:
  /// \brief CodeGen info about outer OpenMP region.
  CodeGenFunction::CGCapturedStmtInfo *OldCSI;
  const OMPExecutableDirective &D;
};

/// \brief RAII for emitting code of OpenMP constructs.
class InlinedOpenMPRegionRAII {
  CodeGenFunction &CGF;

public:
  /// \brief Constructs region for combined constructs.
  /// \param CodeGen Code generation sequence for combined directives. Includes
  /// a list of functions used for code generation of implicitly inlined
  /// regions.
  InlinedOpenMPRegionRAII(CodeGenFunction &CGF, const OMPExecutableDirective &D)
      : CGF(CGF) {
    // Start emission for the construct.
    CGF.CapturedStmtInfo = new CGOpenMPRegionInfo(CGF.CapturedStmtInfo, D);
  }
  ~InlinedOpenMPRegionRAII() {
    // Restore original CapturedStmtInfo only if we're done with code emission.
    auto *OldCSI =
        static_cast<CGOpenMPRegionInfo *>(CGF.CapturedStmtInfo)->getOldCSI();
    delete CGF.CapturedStmtInfo;
    CGF.CapturedStmtInfo = OldCSI;
  }
};

} // namespace

void CodeGenFunction::EmitIntelOpenMPDirective(
    const OMPExecutableDirective &S) {
  OpenMPCodeOutliner Outliner(*this);
  bool DumpClauses = true;
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
    DumpClauses = false;
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
    DumpClauses = false;
    Outliner.emitOMPSingleDirective();
    break;
  case OMPD_master:
    DumpClauses = false;
    Outliner.emitOMPMasterDirective();
    break;
  case OMPD_critical:
    DumpClauses = false;
    Outliner.emitOMPCriticalDirective();
    break;
  case OMPD_ordered:
    DumpClauses = false;
    Outliner.emitOMPOrderedDirective();
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
  case OMPD_target:
  case OMPD_teams:
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
    break;
  case OMPD_threadprivate:
  case OMPD_declare_reduction:
  case OMPD_declare_simd:
  case OMPD_unknown:
    llvm_unreachable("Wrong OpenMP directive");
  }
  if (DumpClauses)
    Outliner << S.clauses();
  if (S.hasAssociatedStmt()) {
    InlinedOpenMPRegionRAII Region(*this, S);
    CapturedStmtInfo->EmitBody(*this, S.getAssociatedStmt());
  }
}
#endif // INTEL_SPECIFIC_OPENMP
