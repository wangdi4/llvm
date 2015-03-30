#ifdef INTEL_SPECIFIC_IL0_BACKEND
#include "clang/Sema/SemaInternal.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Expr.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/Lookup.h"
#include "llvm/ADT/StringExtras.h"
#include "clang/AST/ExprCXX.h"

using namespace clang;

static StringRef CreateStringRef(const char *Str) {
  char *Data = new char[::strlen(Str) + 1];
  ::strcpy(Data, Str);
  return StringRef(Data);
}

static Expr *CreateStringExpr(const char *Str, ASTContext &Context, SourceLocation Loc = SourceLocation()) {
  StringRef Par = CreateStringRef(Str);
  QualType Type = Context.getConstantArrayType(Context.CharTy, 
    llvm::APInt(32, Par.size() + 1), ArrayType::Normal, 0);
  return (StringLiteral::Create(Context, Par, StringLiteral::Ascii, false, Type, Loc));
}

static Expr *CreateStringExpr(const std::string &Str, ASTContext &Context, SourceLocation Loc) {
  return (CreateStringExpr(Str.data(), Context, Loc));
}

static Expr *CreateIntExpr(const int Val, ASTContext &Context, SourceLocation Loc) {
  return (IntegerLiteral::Create(Context, llvm::APInt(32, Val), Context.IntTy, Loc));
}

static void MarkDeclarationsReferencedInPragma(Sema &S, PragmaStmt *P) {
  for (PragmaAttribsVector::iterator I = P->getAttribs().begin(),
                                     E = P->getAttribs().end();
       I != E; ++I) {
    if (I->Value)
      S.MarkDeclarationsReferencedInExpr(I->Value);
  }
  for (PragmaRealAttribsVector::iterator I = P->getRealAttribs().begin(),
                                         E = P->getRealAttribs().end();
       I != E; ++I) {
    if (*I)
      S.MarkDeclarationsReferencedInExpr(*I);
  }
}

// #pragma ivdep
StmtResult Sema::ActOnPragmaOptionsIvdep(SourceLocation KindLoc, IntelPragmaIvdepOption Opt) {
  PragmaStmt* stmt = new (Context) PragmaStmt(KindLoc);
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("IVDEP", Context, KindLoc), IntelPragmaExprConst));
  (stmt->getRealAttribs()).push_back(CreateStringExpr("ivdep", Context, KindLoc));
  if (Opt == IntelPragmaIvdepOptionLoop) {
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("LOOP", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getRealAttribs()).push_back(CreateStringExpr(" loop", Context, KindLoc));
  }
  stmt->setPragmaKind(IntelPragmaIvdep);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

// #pragma novector
StmtResult Sema::ActOnPragmaOptionsNoVector(SourceLocation KindLoc) {
  PragmaStmt* stmt = new (Context) PragmaStmt(KindLoc);
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("LOOP_VECTOR", Context, KindLoc), IntelPragmaExprConst));
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("NEVER", Context, KindLoc), IntelPragmaExprConst));
  (stmt->getRealAttribs()).push_back(CreateStringExpr("novector", Context, KindLoc));
  stmt->setPragmaKind(IntelPragmaNoVector);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

// #pragma distribute_point
StmtResult Sema::ActOnPragmaOptionsDistribute(SourceLocation KindLoc) {
  PragmaStmt* stmt = new (Context) PragmaStmt(KindLoc);
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("DISTRIBUTE", Context, KindLoc), IntelPragmaExprConst));
  (stmt->getRealAttribs()).push_back(CreateStringExpr("distribute_point", Context, KindLoc));
  stmt->setPragmaKind(IntelPragmaDistribute);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

// #pragma inline
StmtResult Sema::ActOnPragmaOptionsInline(SourceLocation KindLoc, 
  IntelPragmaInlineKind PragmaKind, IntelPragmaInlineOption Option) {
  PragmaStmt* stmt = new (Context) PragmaStmt(KindLoc);
  switch (PragmaKind) {
    case (IntelPragmaForceInline):
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("FORCEINLINE", Context, KindLoc), IntelPragmaExprConst));
      (stmt->getRealAttribs()).push_back(CreateStringExpr("forceinline", Context, KindLoc));
      break;
    case (IntelPragmaNoInline):
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("NOINLINE", Context, KindLoc), IntelPragmaExprConst));
      (stmt->getRealAttribs()).push_back(CreateStringExpr("noinline", Context, KindLoc));
      break;
    default:
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("INLINE", Context, KindLoc), IntelPragmaExprConst));
      (stmt->getRealAttribs()).push_back(CreateStringExpr("inline", Context, KindLoc));
      break;
  }
  if (Option == IntelPragmaInlineOptionRecursive) {
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("COMPLETE", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getRealAttribs()).push_back(CreateStringExpr(" recursive", Context, KindLoc));
  }
  stmt->setPragmaKind(IntelPragmaInline);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

StmtResult Sema::ActOnPragmaOptionsEndInline(SourceLocation KindLoc) {
  PragmaStmt* stmt = new (Context) PragmaStmt(KindLoc);
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("ENDINLINE", Context, KindLoc), IntelPragmaExprConst));
  stmt->setPragmaKind(IntelPragmaInlineEnd);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

// #pragma loop_count
StmtResult Sema::ActOnPragmaOptionsLoopCount(SourceLocation KindLoc, 
  const SmallVector<ExprResult, 4> &MinAvgMax,
  const SmallVector<ExprResult, 4> &Regular) {
  PragmaStmt* stmt = new (Context) PragmaStmt(KindLoc);

  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("LOOP_COUNT", Context, KindLoc), IntelPragmaExprConst));
  if (Regular.size() > 0) {
    (stmt->getRealAttribs()).push_back(CreateStringExpr("loop_count (", Context, KindLoc));
    Expr *Comma = CreateStringExpr(", ", Context, KindLoc);
    for (size_t i = 0; i < Regular.size(); ++i) {
      llvm::APSInt Res;
      if (Regular[i].isUsable() && (!Regular[i].get()->isIntegerConstantExpr(Res, Context) || Res.isNegative())) {
        Diag(KindLoc, diag::x_error_intel_pragma_loop_count) << KindLoc;
        DeletePragmaOnError(stmt);
        return (StmtError());
      }
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("AMOUNT", Context, KindLoc), IntelPragmaExprConst));
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(Regular[i].get(), IntelPragmaExprConst));
      (stmt->getRealAttribs()).push_back(Regular[i].get());
      (stmt->getRealAttribs()).push_back(Comma);
    }
    (stmt->getRealAttribs()).back() = CreateStringExpr(")", Context, KindLoc);
  }
  else if (MinAvgMax.size() > 0) {
    for (size_t i = 0; i < MinAvgMax.size(); ++i) {
      llvm::APSInt Res;
      if (MinAvgMax[i].isUsable() && (!MinAvgMax[i].get()->isIntegerConstantExpr(Res, Context) || Res.isNegative())) {
        Diag(KindLoc, diag::x_error_intel_pragma_loop_count) << KindLoc;
        DeletePragmaOnError(stmt);
        return (StmtError());
      }
    }
    (stmt->getRealAttribs()).push_back(CreateStringExpr("loop_count", Context, KindLoc));
    if (MinAvgMax[0].isUsable()) {
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("MIN", Context, KindLoc), IntelPragmaExprConst));
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(MinAvgMax[0].get(), IntelPragmaExprConst));
      (stmt->getRealAttribs()).push_back(CreateStringExpr(" min(", Context, KindLoc));
      (stmt->getRealAttribs()).push_back(MinAvgMax[0].get());
      (stmt->getRealAttribs()).push_back(CreateStringExpr(")", Context, KindLoc));
    }
    if (MinAvgMax[1].isUsable()) {
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("AVG", Context, KindLoc), IntelPragmaExprConst));
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(MinAvgMax[1].get(), IntelPragmaExprConst));
      (stmt->getRealAttribs()).push_back(CreateStringExpr(" avg(", Context, KindLoc));
      (stmt->getRealAttribs()).push_back(MinAvgMax[1].get());
      (stmt->getRealAttribs()).push_back(CreateStringExpr(")", Context, KindLoc));
    }
    if (MinAvgMax[2].isUsable()) {
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("MAX", Context, KindLoc), IntelPragmaExprConst));
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(MinAvgMax[2].get(), IntelPragmaExprConst));
      (stmt->getRealAttribs()).push_back(CreateStringExpr(" max(", Context, KindLoc));
      (stmt->getRealAttribs()).push_back(MinAvgMax[2].get());
      (stmt->getRealAttribs()).push_back(CreateStringExpr(")", Context, KindLoc));
    }
  }

  stmt->setPragmaKind(IntelPragmaLoopCount);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

// #pragma optimize
StmtResult Sema::ActOnPragmaOptionsOptimize(SourceLocation KindLoc, 
  IntelPragmaOptimizeOption Kind) {
  PragmaStmt *stmt = new (Context) PragmaStmt(KindLoc);

  switch (Kind) {
    case (IntelPragmaOptimizeOptionOff):
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("NOOPTIMIZE", Context, KindLoc), IntelPragmaExprConst));
      (stmt->getRealAttribs()).push_back(CreateStringExpr("optimize (\"\", off)", Context, KindLoc));
      break;
    default:
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("OPTIMIZE", Context, KindLoc), IntelPragmaExprConst));
      (stmt->getRealAttribs()).push_back(CreateStringExpr("optimize (\"\", on)", Context, KindLoc));
      break;
  }

  stmt->setPragmaKind(IntelPragmaOptimize);
  if (CommonFunctionOptions.count("OPTIMIZE") > 0) {
    OptionsList[CommonFunctionOptions["OPTIMIZE"]] = NULL;
  }
  CommonFunctionOptions["OPTIMIZE"] = OptionsList.size();
  OptionsList.push_back(stmt);

  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

#include "clang/Sema/SemaConsumer.h"

void Sema::ActOnPragmaOptionsOptimize(PragmaStmt *Pragma) {
  //PragmaDecl *decl = PragmaDecl::Create(Context, Context.getTranslationUnitDecl(), Pragma->getSemiLoc());
  PragmaDecl *decl = PragmaDecl::Create(Context, CurContext, Pragma->getSemiLoc());

  MarkDeclarationsReferencedInPragma(*this, Pragma);
  Pragma->setDecl();
  decl->setStmt(Pragma);

  //Context.getTranslationUnitDecl()->addDecl(decl);
  CurContext->addDecl(decl);
  if (CommonFunctionOptions.count("OPTIMIZE") > 0) {
    OptionsList[CommonFunctionOptions["OPTIMIZE"]] = NULL;
  }
  CommonFunctionOptions["OPTIMIZE"] = OptionsList.size();
  OptionsList.push_back(Pragma);
  //Consumer.HandleTopLevelDecl(DeclGroupRef(decl));
}

// #pragma optimization_level
void Sema::ActOnPragmaOptionsOptimizationLevel(SourceLocation KindLoc, const Token &Tok, bool IsIntelPragma) {
  ExprResult OptLevel;
  Expr* OptLevelReal;
  switch (Tok.getKind()) {
    case (tok::numeric_constant): {
        // OptLevel is a digit
        llvm::APSInt Res;
        OptLevel = ActOnNumericConstant(Tok);
        if (OptLevel.isInvalid() || !OptLevel.get()->isIntegerConstantExpr(Res, Context) || Res.isNegative() || Res.getLimitedValue() > 3) {
          Diag(Tok.getLocation(), diag::x_warn_intel_pragma_opt_level) << Tok.getLocation();
          return;
        }
        OptLevelReal = OptLevel.get();
      }
      break;
    default:
      // OptLevel is 'reset'
      OptLevel = CreateStringExpr("RESET", Context, KindLoc);
      OptLevelReal = CreateStringExpr("reset", Context, KindLoc);
      break;
  }
  PragmaStmt *stmt = new (Context) PragmaStmt(KindLoc);
  //PragmaDecl *decl = PragmaDecl::Create(Context, Context.getTranslationUnitDecl(), KindLoc);
  PragmaDecl *decl = PragmaDecl::Create(Context, CurContext, KindLoc);
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr(IsIntelPragma ? "INTEL_OPTIMIZATION_LEVEL" : "GCC_OPTIMIZATION_LEVEL", Context, KindLoc), IntelPragmaExprConst));
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(OptLevel.get(), IntelPragmaExprConst));
  (stmt->getRealAttribs()).push_back(CreateStringExpr(IsIntelPragma ? "intel optimization_level " : "GCC optimization_level ", Context, KindLoc));
  (stmt->getRealAttribs()).push_back(OptLevelReal);

  stmt->setPragmaKind(IsIntelPragma ? IntelPragmaOptimizationLevel : IntelPragmaOptimizationLevelGCC);
  StringRef Key = IsIntelPragma ? "INTEL_OPTIMIZATION_LEVEL" : "GCC_OPTIMIZATION_LEVEL";
  if (CommonFunctionOptions.count(Key) > 0) {
    OptionsList[CommonFunctionOptions[Key]] = NULL;
  }
  CommonFunctionOptions[Key] = OptionsList.size();
  OptionsList.push_back(stmt);

  MarkDeclarationsReferencedInPragma(*this, stmt);
  stmt->setDecl();
  decl->setStmt(stmt);

  //Context.getTranslationUnitDecl()->addDecl(decl);
  CurContext->addDecl(decl);
  //Consumer.HandleTopLevelDecl(DeclGroupRef(decl));
}

// #pragma noparallel
StmtResult Sema::ActOnPragmaOptionsNoParallel(SourceLocation KindLoc) {
  PragmaStmt* stmt = new (Context) PragmaStmt(KindLoc);
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("NOPARALLEL", Context, KindLoc), IntelPragmaExprConst));
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("NEVER", Context, KindLoc), IntelPragmaExprConst));
  (stmt->getRealAttribs()).push_back(CreateStringExpr("novector", Context, KindLoc));
  stmt->setPragmaKind(IntelPragmaNoParallel);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

// #pragma (no)unroll
StmtResult Sema::ActOnPragmaOptionsUnroll(SourceLocation KindLoc, IntelPragmaUnrollKind Kind, ExprResult Opt) {
  PragmaStmt* stmt = new (Context) PragmaStmt(KindLoc);

  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("UNROLL_COUNT", Context, KindLoc), IntelPragmaExprConst));
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("AMOUNT", Context, KindLoc), IntelPragmaExprConst));
  switch (Kind) {
    case (IntelPragmaSimpleUnroll):
      if (Opt.isUsable()) {
        llvm::APSInt Res;
        if (!Opt.get()->isIntegerConstantExpr(Res, Context)) {
          Diag(KindLoc, diag::x_error_intel_pragma_loop_count) << KindLoc;
          DeletePragmaOnError(stmt);
          return (StmtError());
        }
        (stmt->getAttribs()).push_back(IntelPragmaAttrib(Opt.get(), IntelPragmaExprConst));
        (stmt->getRealAttribs()).push_back(CreateStringExpr("unroll ", Context, KindLoc));
        (stmt->getRealAttribs()).push_back(Opt.get());
      }
      else {
        (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateIntExpr(-1, Context, KindLoc), IntelPragmaExprConst));
        (stmt->getRealAttribs()).push_back(CreateStringExpr("unroll", Context, KindLoc));
      }
      break;
    default:
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateIntExpr(0, Context, KindLoc), IntelPragmaExprConst));
      (stmt->getRealAttribs()).push_back(CreateStringExpr("nounroll", Context, KindLoc));
      break;
  }
  stmt->setPragmaKind(IntelPragmaUnroll);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

// #pragma (no)unroll_and_jam
StmtResult Sema::ActOnPragmaOptionsUnrollAndJam(SourceLocation KindLoc, IntelPragmaUnrollAndJamKind Kind, ExprResult Opt) {
  PragmaStmt* stmt = new (Context) PragmaStmt(KindLoc);

  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("UJAM_COUNT", Context, KindLoc), IntelPragmaExprConst));
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("AMOUNT", Context, KindLoc), IntelPragmaExprConst));
  switch (Kind) {
    case (IntelPragmaSimpleUnroll):
      if (Opt.isUsable()) {
        llvm::APSInt Res;
        if (!Opt.get()->isIntegerConstantExpr(Res, Context)) {
          Diag(KindLoc, diag::x_error_intel_pragma_loop_count) << KindLoc;
          DeletePragmaOnError(stmt);
          return (StmtError());
        }
        (stmt->getAttribs()).push_back(IntelPragmaAttrib(Opt.get(), IntelPragmaExprConst));
        (stmt->getRealAttribs()).push_back(CreateStringExpr("unroll_and_jam ", Context, KindLoc));
        (stmt->getRealAttribs()).push_back(Opt.get());
      }
      else {
        (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateIntExpr(-1, Context, KindLoc), IntelPragmaExprConst));
        (stmt->getRealAttribs()).push_back(CreateStringExpr("unroll_and_jam", Context, KindLoc));
      }
      break;
    default:
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateIntExpr(0, Context, KindLoc), IntelPragmaExprConst));
      (stmt->getRealAttribs()).push_back(CreateStringExpr("nounroll_and_jam", Context, KindLoc));
      break;
  }
  stmt->setPragmaKind(IntelPragmaUnrollAndJam);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

// #pragma nofusion
StmtResult Sema::ActOnPragmaOptionsNoFusion(SourceLocation KindLoc) {
  PragmaStmt* stmt = new (Context) PragmaStmt(KindLoc);
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("FUSION", Context, KindLoc), IntelPragmaExprConst));
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("INFO", Context, KindLoc), IntelPragmaExprConst));
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateIntExpr(0, Context, KindLoc), IntelPragmaExprConst));
  (stmt->getRealAttribs()).push_back(CreateStringExpr("nofusion", Context, KindLoc));
  stmt->setPragmaKind(IntelPragmaNoFusion);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

// #pragma vector
StmtResult Sema::ActOnPragmaOptionsVector(Scope *S, SourceLocation KindLoc, int Opt, 
  const SmallVector<ExprResult, 4> &Exprs) {
  PragmaStmt* stmt = new (Context) PragmaStmt(KindLoc);
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("LOOP_VECTOR", Context, KindLoc), IntelPragmaExprConst));
  (stmt->getRealAttribs()).push_back(CreateStringExpr("vector", Context, KindLoc));
  if (Opt & IntelPragmaVectorAlways) {
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("ALWAYS", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getRealAttribs()).push_back(CreateStringExpr(" always", Context, KindLoc));
  }
  if (Opt & IntelPragmaVectorAssert) {
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("ASSERT", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getRealAttribs()).push_back(CreateStringExpr(" assert", Context, KindLoc));
  }
  if (Opt & IntelPragmaVectorAligned) {
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("ALIGN", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getRealAttribs()).push_back(CreateStringExpr(" aligned", Context, KindLoc));
  }
  if (Opt & IntelPragmaVectorUnAligned) {
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("UNALIGN", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getRealAttribs()).push_back(CreateStringExpr(" unaligned", Context, KindLoc));
  }
  if (Opt & IntelPragmaVectorMaskReadWrite) {
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("MASK_READWRITE", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getRealAttribs()).push_back(CreateStringExpr(" mask_readwrite", Context, KindLoc));
  }
  if (Opt & IntelPragmaVectorNoMaskReadWrite) {
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("NOMASK_READWRITE", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getRealAttribs()).push_back(CreateStringExpr(" nomask_readwrite", Context, KindLoc));
  }
  if (Opt & IntelPragmaVectorVecRemainder) {
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("VEC_REMAINDER", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getRealAttribs()).push_back(CreateStringExpr(" vecremainder", Context, KindLoc));
  }
  if (Opt & IntelPragmaVectorNoVecRemainder) {
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("NOVEC_REMAINDER", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getRealAttribs()).push_back(CreateStringExpr(" novecremainder", Context, KindLoc));
  }
  if (Opt & IntelPragmaVectorTemporal) {
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("TEMPORAL", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getRealAttribs()).push_back(CreateStringExpr(" temporal", Context, KindLoc));
  }
  if (Opt & IntelPragmaVectorNonTemporal) {
    Expr *MainAttrExpr = CreateStringExpr("NONTEMPORAL", Context, KindLoc);
    Expr *RealAttrExpr = CreateStringExpr(" nontemporal", Context, KindLoc);
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(MainAttrExpr, IntelPragmaExprConst));
    if (!Exprs.empty()) {
      (stmt->getRealAttribs()).push_back(RealAttrExpr);
      (stmt->getRealAttribs()).push_back(CreateStringExpr("(", Context, KindLoc));
      for (SmallVector<ExprResult, 4>::const_iterator iter = Exprs.begin(); iter != Exprs.end(); ++iter) {
        Expr *Res = iter->get();
        if (iter->isUsable() && Res->isLValue() && 
          (Res->getType().getTypePtr()->isScalarType() || Res->getType().getTypePtr()->isArrayType())) {
          switch (Res->ClassifyModifiable(Context, KindLoc).getModifiable()) {
            case (Expr::Classification::CM_Modifiable):
              if (Res->getType().getTypePtr()->isScalarType()) {
                (stmt->getAttribs()).push_back(IntelPragmaAttrib(Res, IntelPragmaExprLValue));
                (stmt->getRealAttribs()).push_back(Res);
                (stmt->getRealAttribs()).push_back(CreateStringExpr(", ", Context, KindLoc));
              }
              else
                Diag(Res->getLocStart(), diag::x_warn_intel_pragma_invalid_expr) << Res->getLocStart();
              break;
            case (Expr::Classification::CM_ArrayType): {
                Expr *SubExpr = ActOnArraySubscriptExpr(S, Res, Res->getLocStart(),
                  CreateIntExpr(0, Context, SourceLocation()), SourceLocation()).get();
                if ((SubExpr->getType().getTypePtr()->isScalarType() || SubExpr->getType().getTypePtr()->isArrayType()) &&
                  SubExpr->ClassifyModifiable(Context, KindLoc).getModifiable() == Expr::Classification::CM_Modifiable) {
                  (stmt->getAttribs()).push_back(IntelPragmaAttrib(SubExpr, IntelPragmaExprLValue));
                  (stmt->getRealAttribs()).push_back(Res);
                  (stmt->getRealAttribs()).push_back(CreateStringExpr(", ", Context, KindLoc));
                }
                else {
                  Diag(Res->getLocStart(), diag::x_warn_intel_pragma_invalid_expr) << Res->getLocStart();
                }
              }
              break;
            default:
              Diag(Res->getLocStart(), diag::x_warn_intel_pragma_invalid_expr) << Res->getLocStart();
              break;
          }
        }
        else if (Res) {
          Diag(Res->getLocStart(), diag::x_warn_intel_pragma_invalid_expr) << Res->getLocStart();
        }
      }
      if (stmt->getRealAttribs().size() == 2) {
        (stmt->getRealAttribs()).pop_back();
      }
      else {
        (stmt->getRealAttribs()).back() = CreateStringExpr(")", Context, KindLoc);
      }
    }
    else {
      (stmt->getRealAttribs()).push_back(RealAttrExpr);
    }
  }
  stmt->setPragmaKind(IntelPragmaVector);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

// #pragma optimization_parameter
void Sema::ActOnPragmaOptionsOptimizationParameter(SourceLocation KindLoc, const StringRef &CPU) {
  PragmaStmt *stmt = new (Context) PragmaStmt(KindLoc);
  PragmaDecl *decl = PragmaDecl::Create(Context, CurContext, KindLoc);
  Expr *CPUArch = CreateStringExpr(CPU.data(), Context, KindLoc);
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("OPT_PARAM_TARGET_ARCH", Context, KindLoc), IntelPragmaExprConst));
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CPUArch, IntelPragmaExprConst));
  (stmt->getRealAttribs()).push_back(CreateStringExpr("intel optimization_parameter target_arch = ", Context, KindLoc));
  (stmt->getRealAttribs()).push_back(CPUArch);

  stmt->setPragmaKind(IntelPragmaOptimizationParameter);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  stmt->setDecl();
  decl->setStmt(stmt);

  CurContext->addDecl(decl);
  //Consumer.HandleTopLevelDecl(DeclGroupRef(decl));
  if (CommonFunctionOptions.count("OPT_PARAM_TARGET_ARCH") > 0) {
    OptionsList[CommonFunctionOptions["OPT_PARAM_TARGET_ARCH"]] = NULL;
  }
  CommonFunctionOptions["OPT_PARAM_TARGET_ARCH"] = OptionsList.size();
  OptionsList.push_back(stmt);
}

// #pragma parallel
static ExprResult CreateFunctionRefExpr(Sema &S, FunctionDecl *Fn) {
  DeclRefExpr *DRE = new (S.Context) DeclRefExpr(Fn, false, Fn->getType(),
    VK_LValue, SourceLocation(), DeclarationNameLoc());
  ExprResult E = DRE;
  E = S.DefaultFunctionArrayConversion(E.get());
  if (E.isInvalid()) return ExprError();
  return E;
}

FunctionDecl *Sema::GenerateWrapperDefaultConstructor(SourceLocation Loc, CXXConstructorDecl *ElemFun, const Type *ElemType) {
  std::string Name = std::string(".wrapper_ctor.")
    + llvm::utostr(SourceMgr.getLocForStartOfFile(SourceMgr.getFileID(Loc)).getRawEncoding())
    + "_" + ElemFun->getDeclName().getAsString();
  IdentifierInfo &II = Context.Idents.get(Name);
  DeclarationName DN(&II);
  DeclContext::lookup_result Result = Context.getTranslationUnitDecl()->lookup(DN);
  FunctionDecl *decl;
  if (!Result.empty()) {
    assert (isa<FunctionDecl>(Result.front()) && "FunctionDecl is expected!");
    return cast<FunctionDecl>(Result.front());
  }

  ParmVarDecl *params[1];
  QualType FirstType = Context.getPointerType(QualType(ElemType, 0));
  QualType types[] = {FirstType};
  QualType FunType = Context.getFunctionType(Context.VoidTy, types, FunctionProtoType::ExtProtoInfo());
  MarkDeclarationsReferencedInType(Loc, FunType);

  decl = FunctionDecl::Create(Context, Context.getTranslationUnitDecl(), SourceLocation(),
    SourceLocation(), DN, FunType, 0, SC_Extern, SC_Extern, false, true);
  decl->addAttr(new (Context) NoInlineAttr(SourceRange(), Context, 0));
  decl->setImplicit();
  IdentifierInfo &II1 = Context.Idents.get(StringRef("this"));
  params[0] = ParmVarDecl::Create(Context, decl, SourceLocation(), SourceLocation(), &II1, FirstType,
    0, SC_None, 0);
  params[0]->setScopeInfo(0, 0);
  decl->setParams(params);
  Expr *This = DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(), params[0], false,
        SourceLocation(), params[0]->getType(), VK_RValue);
  MarkDeclarationsReferencedInExpr(This);
  PragmaStmt *PS = new (Context) PragmaStmt(SourceLocation());
  PS->setPragmaKind(IntelPragma_SPECCALL);
  (PS->getAttribs()).push_back(IntelPragmaAttrib(
    CXXConstructExpr::Create(Context, FirstType, SourceLocation(),
      ElemFun, false, llvm::ArrayRef<Expr *>(), false, false, false, false, CXXConstructExpr::CK_Complete, SourceRange()),
    IntelPragmaExprRValue));
  (PS->getAttribs()).push_back(IntelPragmaAttrib(
    This,
    IntelPragmaExprRValue));
  MarkDeclarationsReferencedInPragma(*this, PS);
  Stmt *WCSStmt[1] = {PS};
  decl->setBody(new (Context) CompoundStmt(Context, WCSStmt, SourceLocation(), SourceLocation()));
  MarkFunctionReferenced(Loc, decl);
  decl->setIsUsed();

  Context.getTranslationUnitDecl()->addDecl(decl);
  Consumer.HandleTopLevelDecl(DeclGroupRef(decl));

  return (decl);
}

FunctionDecl *Sema::GenerateWrapperCopyConstructor(SourceLocation Loc, CXXConstructorDecl *ElemFun, const Type *ElemType) {
  std::string Name = std::string(".wrapper_cctor.") 
    + llvm::utostr(SourceMgr.getLocForStartOfFile(SourceMgr.getFileID(Loc)).getRawEncoding()) 
    + "_" + ElemFun->getDeclName().getAsString();
  IdentifierInfo &II = Context.Idents.get(Name);
  DeclarationName DN(&II);
  DeclContext::lookup_result Result = Context.getTranslationUnitDecl()->lookup(DN);
  FunctionDecl *decl;
  if (!Result.empty()) {
    assert (isa<FunctionDecl>(Result.front()) && "FunctionDecl is expected!");
    return cast<FunctionDecl>(Result.front());
  }

  ParmVarDecl *params[2];
  QualType FirstType = Context.getPointerType(QualType(ElemType, 0));
  QualType types[] = {FirstType, FirstType};
  QualType FunType = Context.getFunctionType(Context.VoidTy, types, FunctionProtoType::ExtProtoInfo());
  MarkDeclarationsReferencedInType(Loc, FunType);

  decl = FunctionDecl::Create(Context, Context.getTranslationUnitDecl(), SourceLocation(), 
    SourceLocation(), DN, FunType, 0, SC_Extern, SC_Extern, false, true);
  decl->addAttr(new (Context) NoInlineAttr(SourceRange(), Context, 0));
  decl->setImplicit();
  IdentifierInfo &II1 = Context.Idents.get(StringRef("this"));
  params[0] = ParmVarDecl::Create(Context, decl, SourceLocation(), SourceLocation(), &II1, FirstType, 
    0, SC_None, 0);
  params[0]->setScopeInfo(0, 0);
  IdentifierInfo &II2 = Context.Idents.get(StringRef("src"));
  params[1] = ParmVarDecl::Create(Context, decl, SourceLocation(), SourceLocation(), &II2, FirstType, 
    0, SC_None, 0);
  params[1]->setScopeInfo(0, 1);
  decl->setParams(params);
  Expr *This = DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(), params[0], false,
        SourceLocation(), params[0]->getType(), VK_RValue);
  MarkDeclarationsReferencedInExpr(This);
  Expr *Src = DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(), params[1], false,
        SourceLocation(), params[1]->getType(), VK_RValue);
  MarkDeclarationsReferencedInExpr(Src);
  Expr *Args[] = {Src};
  PragmaStmt *PS = new (Context) PragmaStmt(SourceLocation());
  PS->setPragmaKind(IntelPragma_SPECCALL);
  (PS->getAttribs()).push_back(IntelPragmaAttrib(
    CXXConstructExpr::Create(Context, FirstType, SourceLocation(),
      ElemFun, false, llvm::ArrayRef<Expr *>(Args, 1), false, false, false, false, CXXConstructExpr::CK_Complete, SourceRange()),
    IntelPragmaExprRValue));
  (PS->getAttribs()).push_back(IntelPragmaAttrib(
    This,
    IntelPragmaExprRValue));
  MarkDeclarationsReferencedInPragma(*this, PS);
  Stmt *WCSStmt[1] = {PS};
  decl->setBody(new (Context) CompoundStmt(Context, WCSStmt, SourceLocation(), SourceLocation()));
  MarkFunctionReferenced(Loc, decl);
  decl->setIsUsed();

  Context.getTranslationUnitDecl()->addDecl(decl);
  Consumer.HandleTopLevelDecl(DeclGroupRef(decl));

  return (decl);
}

FunctionDecl *Sema::GenerateWrapperCopyAssignment(SourceLocation Loc, CXXMethodDecl *ElemFun, const Type *ElemType) {
  std::string Name = std::string(".wrapper_cassign.") 
    + llvm::utostr(SourceMgr.getLocForStartOfFile(SourceMgr.getFileID(Loc)).getRawEncoding()) 
    + "_" + ElemFun->getDeclName().getAsString();
  Name.replace(Name.find('='), 1, "_eq_");
  IdentifierInfo &II = Context.Idents.get(Name);
  DeclarationName DN(&II);
  DeclContext::lookup_result Result = Context.getTranslationUnitDecl()->lookup(DN);
  FunctionDecl *decl;
  if (!Result.empty()) {
    assert (isa<FunctionDecl>(Result.front()) && "FunctionDecl is expected!");
    return cast<FunctionDecl>(Result.front());
  }

  SmallVector<ParmVarDecl *, 4> params;
  SmallVector<QualType, 4> types;
  QualType FirstType = Context.getPointerType(QualType(ElemType, 0));
  types.push_back(FirstType);
  const FunctionProtoType *FPT = ElemFun->getType()->getAs<FunctionProtoType>();
  types.append(FPT->param_type_begin(), FPT->param_type_end());

  QualType FunType = Context.getFunctionType(ElemFun->getReturnType(), types, FunctionProtoType::ExtProtoInfo());
  MarkDeclarationsReferencedInType(Loc, FunType);

  decl = FunctionDecl::Create(Context, Context.getTranslationUnitDecl(), SourceLocation(), 
    SourceLocation(), DN, FunType, 0, SC_Extern, SC_Extern, false, true);
  decl->addAttr(new (Context) NoInlineAttr(SourceRange(), Context, 0));
  decl->setImplicit();
  IdentifierInfo &II1 = Context.Idents.get(StringRef("this"));
  params.push_back(ParmVarDecl::Create(Context, decl, SourceLocation(), SourceLocation(), &II1, types[0], 
    0, SC_None, 0));
  params.back()->setScopeInfo(0, 0);
  params.back()->setReferenced();
  for (size_t i = 0; i < FPT->getNumParams(); ++i) {
    params.push_back(ParmVarDecl::Create(Context, decl, SourceLocation(), SourceLocation(), 
      ElemFun->getParamDecl(i)->getIdentifier(), types[i+1], 
      0, SC_None, 0));
    params.back()->setScopeInfo(0, i + 1);
    params.back()->setReferenced();
  }
  decl->setParams(params);
  SmallVector<Expr *, 4> args;
  {
    ExprValueKind VK = Expr::getValueKindForType(params[0]->getType());
    Expr *ThisExpr = BuildUnaryOp(0, SourceLocation(), UO_Deref, 
      DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(), params[0], false,
        SourceLocation(), params[0]->getType().getNonLValueExprType(Context), VK)).get();
    MarkDeclarationsReferencedInExpr(ThisExpr);
    args.push_back(ThisExpr);
  }
  for (size_t i = 1; i < params.size(); ++i) {
    ExprValueKind VK = Expr::getValueKindForType(params[i]->getType());
    args.push_back(DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(), params[i], false,
      SourceLocation(), params[i]->getType().getNonLValueExprType(Context), VK));
  }

  QualType ResultType = ElemFun->getReturnType();
  ExprValueKind VK = Expr::getValueKindForType(ResultType);
  ResultType = ResultType.getNonLValueExprType(Context);

  Stmt *WCSStmt[1] = {new (Context) ReturnStmt(SourceLocation(), 
    new (Context) CXXOperatorCallExpr(Context, OO_Equal, CreateFunctionRefExpr(*this, ElemFun).get(),
      args, ResultType, VK, SourceLocation(), false),
    0)};

  decl->setBody(new (Context) CompoundStmt(Context, WCSStmt, SourceLocation(), SourceLocation()));
  MarkFunctionReferenced(Loc, decl);
  decl->setIsUsed();

  Context.getTranslationUnitDecl()->addDecl(decl);
  Consumer.HandleTopLevelDecl(DeclGroupRef(decl));

  return (decl);
}

FunctionDecl *Sema::GenerateWrapperDestructor(SourceLocation Loc, CXXDestructorDecl *ElemFun, const Type *ElemType) {
  std::string Name = std::string(".wrapper_dtor.") 
    + llvm::utostr(SourceMgr.getLocForStartOfFile(SourceMgr.getFileID(Loc)).getRawEncoding()) 
    + "_" + ElemFun->getDeclName().getAsString();
  Name.replace(Name.find('~'), 1, "_dt_");
  IdentifierInfo &II = Context.Idents.get(Name);
  DeclarationName DN(&II);
  DeclContext::lookup_result Result = Context.getTranslationUnitDecl()->lookup(DN);
  FunctionDecl *decl;
  if (!Result.empty()) {
    assert (isa<FunctionDecl>(Result.front()) && "FunctionDecl is expected!");
    return cast<FunctionDecl>(Result.front());
  }

  ParmVarDecl *params[1];
  QualType FirstType = Context.getPointerType(QualType(ElemType, 0));
  QualType types[] = {FirstType};
  QualType FunType = Context.getFunctionType(Context.VoidTy, types, FunctionProtoType::ExtProtoInfo());
  MarkDeclarationsReferencedInType(Loc, FunType);

  decl = FunctionDecl::Create(Context, Context.getTranslationUnitDecl(), SourceLocation(), 
    SourceLocation(), DN, FunType, 0, SC_Extern, SC_Extern, false, true);
  decl->addAttr(new (Context) NoInlineAttr(SourceRange(), Context, 0));
  decl->setImplicit();
  IdentifierInfo &II1 = Context.Idents.get(StringRef("this"));
  params[0] = ParmVarDecl::Create(Context, decl, SourceLocation(), SourceLocation(), &II1, FirstType, 
    0, SC_None, 0);
  params[0]->setScopeInfo(0, 0);
  decl->setParams(params);
  Expr *This = DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(), params[0], false,
        SourceLocation(), params[0]->getType(), VK_RValue);
  PragmaStmt *PS = new (Context) PragmaStmt(SourceLocation());
  PS->setPragmaKind(IntelPragma_SPECCALL);
  (PS->getAttribs()).push_back(IntelPragmaAttrib(
    new (Context) CXXMemberCallExpr(Context, 
      MemberExpr::Create(Context, This, true, NestedNameSpecifierLoc(), SourceLocation(), ElemFun, 
        DeclAccessPair::make(ElemFun, AS_public), ElemFun->getNameInfo(), 0, ElemFun->getType(), VK_LValue, OK_Ordinary),
    llvm::ArrayRef<Expr *>(), Context.VoidTy, VK_RValue, SourceLocation()),
    IntelPragmaExprRValue));
  (PS->getAttribs()).push_back(IntelPragmaAttrib(
    This,
    IntelPragmaExprRValue));
  MarkDeclarationsReferencedInPragma(*this, PS);
  Stmt *WCSStmt[1] = {PS};
  decl->setBody(new (Context) CompoundStmt(Context, WCSStmt, SourceLocation(), SourceLocation()));
  MarkFunctionReferenced(Loc, decl);
  decl->setIsUsed();

  Context.getTranslationUnitDecl()->addDecl(decl);
  Consumer.HandleTopLevelDecl(DeclGroupRef(decl));

  return (decl);
}

FunctionDecl *Sema::GenerateArrayDefaultConstructor(SourceLocation Loc, CXXConstructorDecl *ElemFun, const Type *ElemType, QualType ArrayType) {
  ParmVarDecl *params[2];
  QualType FirstType = Context.getPointerType(QualType(ElemType, 0));
  QualType types[] = {FirstType, Context.LongLongTy};
  std::string Name = std::string(".array_ctor.")
    + llvm::utostr(SourceMgr.getLocForStartOfFile(SourceMgr.getFileID(Loc)).getRawEncoding()) 
    + "_" + ElemFun->getDeclName().getAsString();
  IdentifierInfo &II = Context.Idents.get(Name);
  DeclarationName DN(&II);
  DeclContext::lookup_result Result = Context.getTranslationUnitDecl()->lookup(DN);
  FunctionDecl *decl;
  if (!Result.empty()) {
    assert (isa<FunctionDecl>(Result.front()) && "FunctionDecl is expected!");
    return cast<FunctionDecl>(Result.front());
  }

  QualType FunType = Context.getFunctionType(Context.VoidTy, types, FunctionProtoType::ExtProtoInfo());
  MarkDeclarationsReferencedInType(Loc, FunType);

  decl = FunctionDecl::Create(Context, Context.getTranslationUnitDecl(), SourceLocation(), 
    SourceLocation(), DN, FunType, 0, SC_Extern, SC_Extern, false, true);
  decl->addAttr(new (Context) NoInlineAttr(SourceRange(), Context, 0));
  decl->setImplicit();
  IdentifierInfo &II1 = Context.Idents.get(StringRef("t"));
  IdentifierInfo &II2 = Context.Idents.get(StringRef("c"));
  params[0] = ParmVarDecl::Create(Context, decl, SourceLocation(), SourceLocation(), &II1, FirstType, 
    0, SC_None, 0);
  params[0]->setScopeInfo(0, 0);
  params[1] = ParmVarDecl::Create(Context, decl, SourceLocation(), SourceLocation(), &II2, Context.LongLongTy, 
    0, SC_None, 0);
  params[1]->setScopeInfo(0, 1);
  decl->setParams(params);
  Expr *array_constr1 = DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(), params[0], false,
        SourceLocation(), params[0]->getType(), VK_RValue);
  Expr *array_constr2 = DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(), params[1], false,
        SourceLocation(), params[1]->getType(), VK_RValue);
  PragmaStmt *PS = new (Context) PragmaStmt(SourceLocation());
  PS->setPragmaKind(IntelPragma_SPECCALLAGG);
  (PS->getAttribs()).push_back(IntelPragmaAttrib(
    CXXConstructExpr::Create(Context, ArrayType, SourceLocation(),
      ElemFun, false, llvm::ArrayRef<Expr *>(), false, false, false, false, CXXConstructExpr::CK_Complete, SourceRange()),
    IntelPragmaExprRValue));
  (PS->getAttribs()).push_back(IntelPragmaAttrib(
    array_constr1,
    IntelPragmaExprRValue));
  (PS->getAttribs()).push_back(IntelPragmaAttrib(
    array_constr2,
    IntelPragmaExprRValue));
  MarkDeclarationsReferencedInPragma(*this, PS);
  Stmt *WCSStmt[1] = {PS};
  decl->setBody(new (Context) CompoundStmt(Context, WCSStmt, SourceLocation(), SourceLocation()));
  MarkFunctionReferenced(Loc, decl);
  decl->setIsUsed();

  Context.getTranslationUnitDecl()->addDecl(decl);
  Consumer.HandleTopLevelDecl(DeclGroupRef(decl));

  return (decl);
}

FunctionDecl *Sema::GenerateArrayCopyConstructor(SourceLocation Loc, CXXConstructorDecl *ElemFun, const Type *ElemType, QualType ArrayType, bool SizeFromType) {
  ParmVarDecl *params[2];
  QualType FirstType = Context.getPointerType(QualType(ElemType, 0));
  QualType types[] = {FirstType, FirstType};
  std::string Name = std::string(".array_cctor.")
    + llvm::utostr(SourceMgr.getLocForStartOfFile(SourceMgr.getFileID(Loc)).getRawEncoding()) 
    + "_" + ElemFun->getDeclName().getAsString();
  IdentifierInfo &II = Context.Idents.get(Name);
  DeclarationName DN(&II);
  DeclContext::lookup_result Result = Context.getTranslationUnitDecl()->lookup(DN);
  FunctionDecl *decl;
  if (!Result.empty()) {
    assert (isa<FunctionDecl>(Result.front()) && "FunctionDecl is expected!");
    return cast<FunctionDecl>(Result.front());
  }

  QualType FunType = Context.getFunctionType(Context.VoidTy, types, FunctionProtoType::ExtProtoInfo());
  MarkDeclarationsReferencedInType(Loc, FunType);

  decl = FunctionDecl::Create(Context, Context.getTranslationUnitDecl(), SourceLocation(), 
    SourceLocation(), DN, FunType, 0, SC_Extern, SC_Extern, false, true);
  decl->addAttr(new (Context) NoInlineAttr(SourceRange(), Context, 0));
  decl->setImplicit();
  IdentifierInfo &II1 = Context.Idents.get(StringRef("t"));
  IdentifierInfo &II2 = Context.Idents.get(StringRef("s"));
  params[0] = ParmVarDecl::Create(Context, decl, SourceLocation(), SourceLocation(), &II1, FirstType, 
    0, SC_None, 0);
  params[0]->setScopeInfo(0, 0);
  params[1] = ParmVarDecl::Create(Context, decl, SourceLocation(), SourceLocation(), &II2, FirstType, 
    0, SC_None, 0);
  params[1]->setScopeInfo(0, 1);
  decl->setParams(params);
  Expr *array_constr1 = DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(), params[0], false,
        SourceLocation(), params[0]->getType(), VK_RValue);
  Expr *array_constr2 = DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(), params[1], false,
        SourceLocation(), params[1]->getType(), VK_RValue);

  // Class *src;
  IdentifierInfo &II4 = Context.Idents.get(StringRef("src"));
  VarDecl *srcDecl = VarDecl::Create(Context, decl, SourceLocation(), SourceLocation(),
    &II4, FirstType, 0, SC_Auto);
  Expr *Src = DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(),
    srcDecl, true, SourceLocation(), srcDecl->getType(), VK_LValue);

  Stmt *DC1 = new (Context) DeclStmt(DeclGroupRef(srcDecl), SourceLocation(), SourceLocation());
  // src = s;
  ExprResult assignOp1 = ImpCastExprToType(BuildBinOp(0, SourceLocation(), BO_Assign, Src, array_constr2).get(),
    Context.VoidTy, CK_ToVoid);


  // src++
  ExprResult SrcInc = BuildUnaryOp(0, SourceLocation(),
    UO_Deref, BuildUnaryOp(0, SourceLocation(), UO_PostInc, Src).get());
  Expr *Args[1] = {SrcInc.get()};

  Expr *Size;
  if (!SizeFromType) {
    Size = ActOnIntegerConstant(SourceLocation(), 0).get();
  }
  else if (ArrayType->isConstantArrayType()) {
    Size = new (Context) IntegerLiteral(Context, cast<ConstantArrayType>(ArrayType.getTypePtr())->getSize(), 
      Context.LongLongTy, SourceLocation());
  }
  else if (ArrayType->isDependentSizedArrayType()) {
    Size = cast<DependentSizedArrayType>(ArrayType.getTypePtr())->getSizeExpr();
  }
  else if (ArrayType->isVariableArrayType()) {
    Size = cast<VariableArrayType>(ArrayType.getTypePtr())->getSizeExpr();
  }
  else {
    Size = ActOnIntegerConstant(SourceLocation(), 0).get();
  }

  PragmaStmt *PS = new (Context) PragmaStmt(SourceLocation());
  PS->setPragmaKind(IntelPragma_SPECCALLAGG);
  (PS->getAttribs()).push_back(IntelPragmaAttrib(
    CXXConstructExpr::Create(Context, ArrayType, SourceLocation(),
      ElemFun, false, llvm::ArrayRef<Expr *>(Args, 1), false, false, false, false, CXXConstructExpr::CK_Complete, SourceRange()),
    IntelPragmaExprRValue));
  (PS->getAttribs()).push_back(IntelPragmaAttrib(
    array_constr1,
    IntelPragmaExprRValue));
  (PS->getAttribs()).push_back(IntelPragmaAttrib(
    Size,
    IntelPragmaExprRValue));
  MarkDeclarationsReferencedInPragma(*this, PS);
  Stmt *WCSStmt[3] = {DC1, assignOp1.get(), PS};
  decl->setBody(new (Context) CompoundStmt(Context, WCSStmt, SourceLocation(), SourceLocation()));
  MarkFunctionReferenced(Loc, decl);
  decl->setIsUsed();

  Context.getTranslationUnitDecl()->addDecl(decl);
  Consumer.HandleTopLevelDecl(DeclGroupRef(decl));

  return (decl);
}

FunctionDecl *Sema::GenerateArrayCopyAssignment(SourceLocation Loc, CXXMethodDecl *ElemFun, const Type *ElemType, QualType ArrayType, bool SizeFromType) {
  ParmVarDecl *params[2];
  QualType FirstType = Context.getPointerType(QualType(ElemType, 0));
  QualType types[] = {FirstType, FirstType};
  std::string Name = std::string(".array_cassign.")
    + llvm::utostr(SourceMgr.getLocForStartOfFile(SourceMgr.getFileID(Loc)).getRawEncoding()) 
    + "_" + ElemFun->getDeclName().getAsString();
  Name.replace(Name.find('='), 1, "_eq_");
  IdentifierInfo &II = Context.Idents.get(Name);
  DeclarationName DN(&II);
  DeclContext::lookup_result Result = Context.getTranslationUnitDecl()->lookup(DN);
  FunctionDecl *decl;
  if (!Result.empty()) {
    assert (isa<FunctionDecl>(Result.front()) && "FunctionDecl is expected!");
    return cast<FunctionDecl>(Result.front());
  }

  QualType FunType = Context.getFunctionType(Context.VoidTy, types, FunctionProtoType::ExtProtoInfo());
  MarkDeclarationsReferencedInType(Loc, FunType);

  decl = FunctionDecl::Create(Context, Context.getTranslationUnitDecl(), SourceLocation(), 
    SourceLocation(), DN, FunType, 0, SC_Extern, SC_Extern, false, true);
  decl->addAttr(new (Context) NoInlineAttr(SourceRange(), Context, 0));
  decl->setImplicit();
  
  IdentifierInfo &II1 = Context.Idents.get(StringRef("t"));
  IdentifierInfo &II2 = Context.Idents.get(StringRef("s"));
  params[0] = ParmVarDecl::Create(Context, Context.getTranslationUnitDecl(), SourceLocation(), SourceLocation(), &II1, FirstType, 
    0, SC_None, 0);
  params[0]->setScopeInfo(0, 0);
  params[1] = ParmVarDecl::Create(Context, Context.getTranslationUnitDecl(), SourceLocation(), SourceLocation(), &II2, FirstType, 
    0, SC_None, 0);
  params[1]->setScopeInfo(0, 1);
  decl->setParams(params);

  Expr *par1 = DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(),
    params[0], true, SourceLocation(), params[0]->getType(), VK_RValue);
  Expr *par2 = DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(),
    params[1], true, SourceLocation(), params[1]->getType(), VK_RValue);

  // long long cnt;
  IdentifierInfo &II4 = Context.Idents.get(StringRef("cnt"));
  VarDecl *cntDecl = VarDecl::Create(Context, decl, SourceLocation(), SourceLocation(),
    &II4, Context.LongLongTy, 0, SC_Auto);
  Expr *cnt = DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(),
    cntDecl, true, SourceLocation(), cntDecl->getType(), VK_LValue);
  // Class *this;
  IdentifierInfo &II5 = Context.Idents.get(StringRef("this"));
  VarDecl *thisDecl = VarDecl::Create(Context, decl, SourceLocation(), SourceLocation(),
    &II5, FirstType, 0, SC_Auto);
  Expr *This = DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(),
    thisDecl, true, SourceLocation(), thisDecl->getType(), VK_LValue);
  // Class *src;
  IdentifierInfo &II6 = Context.Idents.get(StringRef("src"));
  VarDecl *srcDecl = VarDecl::Create(Context, decl, SourceLocation(), SourceLocation(),
    &II6, FirstType, 0, SC_Auto);
  Expr *Src = DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(),
    srcDecl, true, SourceLocation(), thisDecl->getType(), VK_LValue);

  Stmt *DC1 = new (Context) DeclStmt(DeclGroupRef(cntDecl), SourceLocation(), SourceLocation());
  Stmt *DC2 = new (Context) DeclStmt(DeclGroupRef(thisDecl), SourceLocation(), SourceLocation());
  Stmt *DC3 = new (Context) DeclStmt(DeclGroupRef(srcDecl), SourceLocation(), SourceLocation());

  Expr *Size;
  if (!SizeFromType) {
    Size = ActOnIntegerConstant(SourceLocation(), 0).get();
  }
  else if (ArrayType->isConstantArrayType()) {
    Size = new (Context) IntegerLiteral(Context, cast<ConstantArrayType>(ArrayType.getTypePtr())->getSize(), 
      Context.LongLongTy, SourceLocation());
  }
  else if (ArrayType->isDependentSizedArrayType()) {
    Size = cast<DependentSizedArrayType>(ArrayType.getTypePtr())->getSizeExpr();
  }
  else if (ArrayType->isVariableArrayType()) {
    Size = cast<VariableArrayType>(ArrayType.getTypePtr())->getSizeExpr();
  }
  else {
    Size = ActOnIntegerConstant(SourceLocation(), 0).get();
  }

  // cnt = size;
  ExprResult assignOp = ImpCastExprToType(BuildBinOp(0, SourceLocation(), BO_Assign, cnt, Size).get(),
    Context.VoidTy, CK_ToVoid);
  // this = t;
  ExprResult assignOp1 = ImpCastExprToType(BuildBinOp(0, SourceLocation(), BO_Assign, This, par1).get(),
    Context.VoidTy, CK_ToVoid);
  // src = s;
  ExprResult assignOp2 = ImpCastExprToType(BuildBinOp(0, SourceLocation(), BO_Assign, Src, par2).get(),
    Context.VoidTy, CK_ToVoid);
  // cnt != 0
  ExprResult cond = BuildBinOp(0, SourceLocation(), BO_NE, cnt, ActOnIntegerConstant(SourceLocation(), 0).get());
  // --cnt;
  ExprResult cntDec = ImpCastExprToType(BuildUnaryOp(0, SourceLocation(), UO_PreDec, cnt).get(),
    Context.VoidTy, CK_ToVoid);
  // this=src++;
  Expr *ThisExpr = BuildUnaryOp(0, SourceLocation(), UO_Deref, This).get();
  ExprResult SrcInc = BuildUnaryOp(0, SourceLocation(),
    UO_Deref, BuildUnaryOp(0, SourceLocation(), UO_PostInc, Src).get());
  Expr *Args[2] = {ThisExpr, SrcInc.get()};

  QualType ResultType = ElemFun->getReturnType();
  ExprValueKind VK = Expr::getValueKindForType(ResultType);
  ResultType = ResultType.getNonLValueExprType(Context);

  Expr *cassign = ImpCastExprToType(new (Context) CXXOperatorCallExpr(Context, OO_Equal, CreateFunctionRefExpr(*this, ElemFun).get(),
      llvm::ArrayRef<Expr *>(Args, 2), ResultType, VK, SourceLocation(), false),
      Context.VoidTy, CK_ToVoid).get();
  // ++this;
  ExprResult array_ptr1Inc = ImpCastExprToType(BuildUnaryOp(0, SourceLocation(), UO_PreInc, This).get(),
    Context.VoidTy, CK_ToVoid);

  //while (cnt != 0) {
  // --cnt;
  // *this=*(src++);
  // ++this;
  //}
  Stmt *BodyStmt[3] = {cntDec.get(), cassign, array_ptr1Inc.get()};
  StmtResult whileStmt = ActOnWhileStmt(SourceLocation(), MakeFullExpr(cond.get()), 0, 
    new (Context) CompoundStmt(Context, BodyStmt, SourceLocation(), SourceLocation()));

  Stmt *WCSStmt[7] = {DC1, DC2, DC3, assignOp.get(), assignOp1.get(), assignOp2.get(), whileStmt.get()};
  decl->setBody(new (Context) CompoundStmt(Context, WCSStmt, SourceLocation(), SourceLocation()));
  MarkFunctionReferenced(Loc, decl);
  decl->setIsUsed();

  Context.getTranslationUnitDecl()->addDecl(decl);
  Consumer.HandleTopLevelDecl(DeclGroupRef(decl));

  return (decl);
}

FunctionDecl *Sema::GenerateArrayDestructor(SourceLocation Loc, CXXDestructorDecl *ElemFun, const Type *ElemType) {
  ParmVarDecl *params[2];
  QualType FirstType = Context.getPointerType(QualType(ElemType, 0));
  QualType types[] = {FirstType, Context.LongLongTy};
  std::string Name = std::string(".array_dtor.")
    + llvm::utostr(SourceMgr.getLocForStartOfFile(SourceMgr.getFileID(Loc)).getRawEncoding()) 
    + "_" + ElemFun->getDeclName().getAsString();
  Name.replace(Name.find('~'), 1, "_dt_");
  IdentifierInfo &II = Context.Idents.get(Name);
  DeclarationName DN(&II);
  DeclContext::lookup_result Result = Context.getTranslationUnitDecl()->lookup(DN);
  FunctionDecl *decl;
  if (!Result.empty()) {
    assert (isa<FunctionDecl>(Result.front()) && "FunctionDecl is expected!");
    return cast<FunctionDecl>(Result.front());
  }

  QualType FunType = Context.getFunctionType(Context.VoidTy, types, FunctionProtoType::ExtProtoInfo());
  MarkDeclarationsReferencedInType(Loc, FunType);

  decl = FunctionDecl::Create(Context, Context.getTranslationUnitDecl(), SourceLocation(), 
    SourceLocation(), DN, FunType, 0, SC_Extern, SC_Extern, false, true);
  decl->addAttr(new (Context) NoInlineAttr(SourceRange(), Context, 0));
  decl->setImplicit();
  
  IdentifierInfo &II1 = Context.Idents.get(StringRef("t"));
  IdentifierInfo &II2 = Context.Idents.get(StringRef("c"));
  params[0] = ParmVarDecl::Create(Context, Context.getTranslationUnitDecl(), SourceLocation(), SourceLocation(), &II1, FirstType, 
    0, SC_None, 0);
  params[0]->setScopeInfo(0, 0);
  params[0]->setImplicit();
  params[1] = ParmVarDecl::Create(Context, Context.getTranslationUnitDecl(), SourceLocation(), SourceLocation(), &II2, Context.LongLongTy, 
    0, SC_None, 0);
  params[1]->setScopeInfo(0, 1);
  params[1]->setImplicit();
  decl->setParams(params);
  params[0]->setLexicalDeclContext(decl);
  params[1]->setLexicalDeclContext(decl);
  decl->addDecl(params[0]);
  decl->addDecl(params[1]);

  Expr *array_destr1 = DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(),
    params[0], true, SourceLocation(), params[0]->getType(), VK_RValue);
  Expr *array_destr2 = DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(),
    params[1], true, SourceLocation(), params[1]->getType(), VK_RValue);

  // long long cnt;
  IdentifierInfo &II3 = Context.Idents.get(StringRef("cnt"));
  VarDecl *cntDecl = VarDecl::Create(Context, decl, SourceLocation(), SourceLocation(),
    &II3, Context.LongLongTy, 0, SC_Auto);
  Expr *cnt = DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(),
    cntDecl, true, SourceLocation(), cntDecl->getType(), VK_LValue);
  // Class *this;
  IdentifierInfo &II4 = Context.Idents.get(StringRef("this"));
  VarDecl *thisDecl = VarDecl::Create(Context, decl, SourceLocation(), SourceLocation(),
    &II4, FirstType, 0, SC_Auto);
  Expr *This = DeclRefExpr::Create(Context, NestedNameSpecifierLoc(), SourceLocation(),
    thisDecl, true, SourceLocation(), thisDecl->getType(), VK_LValue);

  Stmt *DC1 = new (Context) DeclStmt(DeclGroupRef(cntDecl), SourceLocation(), SourceLocation());
  Stmt *DC2 = new (Context) DeclStmt(DeclGroupRef(thisDecl), SourceLocation(), SourceLocation());

  // cnt = c;
  ExprResult assignOp = ImpCastExprToType(BuildBinOp(0, SourceLocation(), BO_Assign, cnt, array_destr2).get(),
    Context.VoidTy, CK_ToVoid);
  // this = t;
  ExprResult assignOp1 = ImpCastExprToType(BuildBinOp(0, SourceLocation(), BO_Assign, This, array_destr1).get(),
    Context.VoidTy, CK_ToVoid);
  // cnt != 0
  ExprResult cond = BuildBinOp(0, SourceLocation(), BO_NE, cnt, ActOnIntegerConstant(SourceLocation(), 0).get());
  // --cnt;
  ExprResult cntDec = ImpCastExprToType(BuildUnaryOp(0, SourceLocation(), UO_PreDec, cnt).get(),
    Context.VoidTy, CK_ToVoid);
  // ~Class(this);
  Expr *destr = new (Context) CXXMemberCallExpr(Context, 
      MemberExpr::Create(Context, This, true, NestedNameSpecifierLoc(), SourceLocation(), ElemFun, 
        DeclAccessPair::make(ElemFun, AS_public), ElemFun->getNameInfo(), 0, ElemFun->getType(), VK_LValue, OK_Ordinary),
    llvm::ArrayRef<Expr *>(), Context.VoidTy, VK_RValue, SourceLocation());
  // ++this;
  ExprResult array_ptr1Inc = ImpCastExprToType(BuildUnaryOp(0, SourceLocation(), UO_PreInc, This).get(),
    Context.VoidTy, CK_ToVoid);

  //while (cnt != 0) {
  // --cnt;
  // ~Class(array_ptr1);
  // ++this;
  //}
  Stmt *BodyStmt[3] = {cntDec.get(), destr, array_ptr1Inc.get()};
  StmtResult whileStmt = ActOnWhileStmt(SourceLocation(), MakeFullExpr(cond.get()), 0, 
    new (Context) CompoundStmt(Context, BodyStmt, SourceLocation(), SourceLocation()));

  Stmt *WCSStmt[5] = {DC1, DC2, assignOp.get(), assignOp1.get(), whileStmt.get()};
  decl->setBody(new (Context) CompoundStmt(Context, WCSStmt, SourceLocation(), SourceLocation()));
  MarkFunctionReferenced(Loc, decl);
  decl->setIsUsed();

  Context.getTranslationUnitDecl()->addDecl(decl);
  Consumer.HandleTopLevelDecl(DeclGroupRef(decl));

  return (decl);
}

bool Sema::ActOnNonPODVariable(SourceLocation Loc, QualType QT, PragmaAttribsVector &Attribs) {
  const Type *BaseType = QT.getTypePtr();

  if (!LangOpts.CPlusPlus) return (true);

  if (BaseType->isStructureOrClassType() || BaseType->isUnionType()) {
    CXXRecordDecl *CXXRD = BaseType->getAsCXXRecordDecl();
    CXXConstructorDecl *DC = LookupDefaultConstructor(CXXRD);
    if (DC && !DC->isTrivial()) {
      // Generate code for default constructor
      MarkAnyDeclReferenced(Loc, DC, true);
      ValueDecl *FD = GenerateWrapperDefaultConstructor(Loc, DC, BaseType);
      DeclRefExpr *expr = new (Context) DeclRefExpr(FD, false, FD->getType(), VK_RValue, Loc);
      Attribs.push_back(IntelPragmaAttrib(CreateStringExpr("CTOR", Context), IntelPragmaExprConst));
      Attribs.push_back(IntelPragmaAttrib(expr, IntelPragmaExprRValue));
      //eee = expr;
      ;
    }
    if (CXXRD && !QT.isTriviallyCopyableType(Context)) {
      CXXMethodDecl *MC = LookupCopyingAssignment(CXXRD, Qualifiers::Const, false, 0);
      if (MC && !MC->isTrivial()) {
        // Generate code for copy constructor
        //printf("WEWEW\n");
        MarkAnyDeclReferenced(Loc, MC, true);
        ValueDecl *FD = GenerateWrapperCopyAssignment(Loc, MC, BaseType);
        DeclRefExpr *expr = new (Context) DeclRefExpr(FD, false, FD->getType(), VK_RValue, Loc);
        Attribs.push_back(IntelPragmaAttrib(CreateStringExpr("CASSIGN", Context), IntelPragmaExprConst));
        Attribs.push_back(IntelPragmaAttrib(expr, IntelPragmaExprRValue));
        //eee2 = expr;
        //GenerateWrapperFunction(MC, BaseType);
        //DeclRefExpr *expr = new (Context) DeclRefExpr(MC, false, MC->getType(), VK_RValue, Loc);
        //eee2 = expr;
        ;
      }
    }
    if (CXXRD) {
      CXXDestructorDecl *DD = LookupDestructor(CXXRD);
      if (DD && !DD->isTrivial()) {
        // Generate code for destructor
        MarkAnyDeclReferenced(Loc, DD, true);
        ValueDecl *FD = GenerateWrapperDestructor(Loc, DD, BaseType);
        DeclRefExpr *expr = new (Context) DeclRefExpr(FD, false, FD->getType(), VK_RValue, Loc);
        Attribs.push_back(IntelPragmaAttrib(CreateStringExpr("DTOR", Context), IntelPragmaExprConst));
        Attribs.push_back(IntelPragmaAttrib(expr, IntelPragmaExprRValue));
        //eee1 = expr;
        //GenerateWrapperFunction(DD, BaseType);
        //DeclRefExpr *expr = new (Context) DeclRefExpr(DD, false, DD->getType(), VK_RValue, Loc);
        //eee1 = expr;
        ;
      }
    }
  }
  else if (BaseType->isArrayType()) {
    const Type *ArrBaseType = BaseType->getArrayElementTypeNoTypeQual();
    CXXRecordDecl *CXXRD = ArrBaseType->getAsCXXRecordDecl();
    if (CXXRD) {
      CXXConstructorDecl *DC = LookupDefaultConstructor(CXXRD);
      if (DC && !DC->isTrivial()) {
        // Generate code for array with default constructor
        MarkAnyDeclReferenced(Loc, DC, true);
        //printf("ArrayDefaultConstr\n");
        ValueDecl *FD = GenerateArrayDefaultConstructor(Loc, DC, ArrBaseType, QT);
        DeclRefExpr *expr = new (Context) DeclRefExpr(FD, false, FD->getType(), VK_RValue, Loc);
        Attribs.push_back(IntelPragmaAttrib(CreateStringExpr("CTOR", Context), IntelPragmaExprConst));
        Attribs.push_back(IntelPragmaAttrib(expr, IntelPragmaExprRValue));
        ;
      }
    }
    if (CXXRD && !Context.getQualifiedType(ArrBaseType, Qualifiers()).isTriviallyCopyableType(Context)) {
      CXXMethodDecl *MC = LookupCopyingAssignment(CXXRD, Qualifiers::Const, false, 0);
      if (MC && !MC->isTrivial()) {
        // Generate code for array with copy assignment
        MarkAnyDeclReferenced(Loc, MC, true);
        //printf("ArrayCopyAssignment\n");
        ValueDecl *FD = GenerateArrayCopyAssignment(Loc, MC, ArrBaseType, QT, false);
        DeclRefExpr *expr = new (Context) DeclRefExpr(FD, false, FD->getType(), VK_RValue, Loc);
        Attribs.push_back(IntelPragmaAttrib(CreateStringExpr("CASSIGN", Context), IntelPragmaExprConst));
        Attribs.push_back(IntelPragmaAttrib(expr, IntelPragmaExprRValue));
        //GenerateArrayFunction(Loc, MC, ArrBaseType);
        ;
      }
    }
    if (CXXRD) {
      CXXDestructorDecl *DD = LookupDestructor(CXXRD);
      if (DD && !DD->isTrivial()) {
        // Generate code for array with destructor
        MarkAnyDeclReferenced(Loc, DD, true);
        //printf("ArrayDestructor\n");
        ValueDecl *FD = GenerateArrayDestructor(Loc, DD, ArrBaseType);
        DeclRefExpr *expr = new (Context) DeclRefExpr(FD, false, FD->getType(), VK_RValue, Loc);
        Attribs.push_back(IntelPragmaAttrib(CreateStringExpr("DTOR", Context), IntelPragmaExprConst));
        Attribs.push_back(IntelPragmaAttrib(expr, IntelPragmaExprRValue));
        //GenerateArrayFunction(Loc, DD, ArrBaseType);
        ;
      }
    }
  }
  return (true);
}

void Sema::CheckAndGenVars(Scope *S, SourceLocation KindLoc, 
  const SmallVector<ExprResult, 4> &VarsExprs, const SmallVector<ExprResult, 4> &SizeVarsExprs, 
  const char *RealAttrib, const char *MainAttrib, PragmaStmt *stmt,
  const SmallVector<ExprResult, 4> &CheckAgainstVarsExprs) {
  Expr *MainAttrExpr = CreateStringExpr(MainAttrib, Context, KindLoc);
  Expr *RealAttrExpr = CreateStringExpr(RealAttrib, Context, KindLoc);
  if (!VarsExprs.empty()) {
    (stmt->getRealAttribs()).push_back(RealAttrExpr);
    (stmt->getRealAttribs()).push_back(CreateStringExpr("(", Context, KindLoc));
    for (SmallVector<ExprResult, 4>::const_iterator iter = VarsExprs.begin(),
      iter1 = SizeVarsExprs.begin(); iter != VarsExprs.end(); ++iter, ++iter1) {
      Expr *Res = NULL;
      if (iter->isUsable() && (Res = iter->get()->IgnoreParenCasts())->isLValue() && (isa<DeclRefExpr>(Res) || 
        (isa<ArraySubscriptExpr>(Res) && isa<DeclRefExpr>(cast<ArraySubscriptExpr>(Res)->getLHS()->IgnoreParenCasts())))) {
        if (iter1->isUsable()) {
          QualType OpType = Res->getType();
          bool IsArray = OpType->isArrayType();
          if (!IsArray && !OpType->isPointerType() && !OpType->isTemplateTypeParmType()) {
            Diag(Res->getLocStart(), diag::x_error_intel_pragma_parallel_pointer_array_is_allowed) << Res->getLocStart();
            continue;
          }
          llvm::APSInt IntSize;
          if (IsArray && OpType->isConstantArrayType() && 
            iter1->get()->EvaluateAsInt(IntSize, Context) && 
            (IntSize.isNegative() 
              || IntSize > llvm::APSInt(cast<ConstantArrayType>(OpType.getTypePtr())->getSize(), IntSize.isUnsigned()).extOrTrunc(IntSize.getBitWidth()))) {
            Diag(Res->getLocStart(), diag::x_error_intel_pragma_parallel_subscript_out_of_range) << Res->getLocStart();
            continue;
          }
        }
        (stmt->getAttribs()).push_back(IntelPragmaAttrib(MainAttrExpr, IntelPragmaExprConst));
        if (isa<DeclRefExpr>(Res)) {
          for (SmallVector<ExprResult, 4>::const_iterator checkIter = CheckAgainstVarsExprs.begin(); 
            checkIter != CheckAgainstVarsExprs.end(); ++checkIter) {
            Expr *CheckExpr;
            if (checkIter->isUsable() && (CheckExpr = checkIter->get()->IgnoreParenCasts())->isLValue() && (isa<DeclRefExpr>(CheckExpr) || 
              (isa<ArraySubscriptExpr>(CheckExpr) && isa<DeclRefExpr>(cast<ArraySubscriptExpr>(CheckExpr)->getLHS()->IgnoreParenCasts()) 
                && (CheckExpr = cast<ArraySubscriptExpr>(CheckExpr)->getLHS()->IgnoreParenCasts())))) {
              if (declaresSameEntity(cast<DeclRefExpr>(CheckExpr)->getDecl(), cast<DeclRefExpr>(Res)->getDecl())) {
                // Error
                //std::string S;
                //cast<ValueDecl>(cast<DeclRefExpr>(Res)->getDecl())->getNameForDiagnostic(S, Context.getPrintingPolicy(), true);
                Diag(Res->getLocStart(), diag::x_error_intel_pragma_parallel_private) << cast<NamedDecl>(cast<DeclRefExpr>(Res)->getDecl())
                   << Res->getLocStart();
                break;
              }
            }
          }
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(Res, IntelPragmaExprLValue));
          ActOnNonPODVariable(Res->getLocStart(), Res->getType(), stmt->getAttribs());
          if (iter1->isUsable()) {
            (stmt->getAttribs()).push_back(IntelPragmaAttrib(iter1->get(), IntelPragmaExprRValue));
          }
        }
        else {
          Expr *Res1 = cast<ArraySubscriptExpr>(Res)->getLHS()->IgnoreParenCasts();
          if (iter1->isUsable()) {
            QualType OpType = Res1->getType();
            bool IsArray = OpType->isArrayType();
            if (!IsArray && !OpType->isPointerType() && !OpType->isTemplateTypeParmType()) {
              Diag(Res->getLocStart(), diag::x_error_intel_pragma_parallel_pointer_array_is_allowed) << Res->getLocStart();
              continue;
            }
            llvm::APSInt IntSize;
            if (IsArray && OpType->isConstantArrayType() && 
              iter1->get()->EvaluateAsInt(IntSize, Context) && 
              (IntSize.isNegative() 
                || IntSize > llvm::APSInt(cast<ConstantArrayType>(OpType.getTypePtr())->getSize(), IntSize.isUnsigned()).extOrTrunc(IntSize.getBitWidth()))) {
              Diag(Res->getLocStart(), diag::x_error_intel_pragma_parallel_subscript_out_of_range) << Res->getLocStart();
              continue;
            }
          }
          for (SmallVector<ExprResult, 4>::const_iterator checkIter = CheckAgainstVarsExprs.begin(); 
            checkIter != CheckAgainstVarsExprs.end(); ++checkIter) {
            Expr *CheckExpr;
            if (checkIter->isUsable() && (CheckExpr = checkIter->get()->IgnoreParenCasts())->isLValue() && (isa<DeclRefExpr>(CheckExpr) || 
              (isa<ArraySubscriptExpr>(CheckExpr) && isa<DeclRefExpr>(cast<ArraySubscriptExpr>(CheckExpr)->getLHS()->IgnoreParenCasts()) 
                && (CheckExpr = cast<ArraySubscriptExpr>(CheckExpr)->getLHS()->IgnoreParenCasts())))) {
              if (declaresSameEntity(cast<DeclRefExpr>(CheckExpr)->getDecl(), cast<DeclRefExpr>(Res1)->getDecl())) {
                // Error
                //std::string S;
                //cast<ValueDecl>(cast<DeclRefExpr>(Res1)->getDecl())->getNameForDiagnostic(S, Context.getPrintingPolicy(), true);
                Diag(Res->getLocStart(), diag::x_error_intel_pragma_parallel_private) << cast<NamedDecl>(cast<DeclRefExpr>(Res1)->getDecl())
                   << Res->getLocStart();
                break;
              }
            }
          }
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(Res1, IntelPragmaExprLValue));
          //(stmt->getAttribs()).push_back(IntelPragmaAttrib(cast<ArraySubscriptExpr>(Res)->getRHS(), IntelPragmaExprRValue));
          ActOnNonPODVariable(Res1->getLocStart(), Res->getType(), stmt->getAttribs());
          if (iter1->isUsable()) {
            (stmt->getAttribs()).push_back(IntelPragmaAttrib(iter1->get(), IntelPragmaExprRValue));
          }
        }
        (stmt->getRealAttribs()).push_back(Res);
        (stmt->getRealAttribs()).push_back(CreateStringExpr(", ", Context, KindLoc));
      }
      else if (Res) {
        Diag(Res->getLocStart(), diag::x_error_intel_pragma_parallel_vars) << Res->getLocStart();
      }
    }
    if (stmt->getRealAttribs().size() == 2) {
      (stmt->getRealAttribs()).pop_back();
    }
    else {
      (stmt->getRealAttribs()).back() = CreateStringExpr(")", Context, KindLoc);
    }
  }
  else {
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(MainAttrExpr, IntelPragmaExprConst));
    (stmt->getRealAttribs()).push_back(RealAttrExpr);
  }
}

StmtResult Sema::ActOnPragmaOptionsParallel(Scope *S, SourceLocation KindLoc, int Opt, 
  const SmallVector<ExprResult, 4> &Private, const SmallVector<ExprResult, 4> &SizePrivate, 
  const SmallVector<ExprResult, 4> &LastPrivate, const SmallVector<ExprResult, 4> &SizeLastPrivate, 
  const SmallVector<ExprResult, 4> &FirstPrivate, const SmallVector<ExprResult, 4> &SizeFirstPrivate,
  ExprResult Collapse, ExprResult NumThreads) {
  PragmaStmt* stmt = new (Context) PragmaStmt(KindLoc);

  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("PARALLEL", Context, KindLoc), IntelPragmaExprConst));
  (stmt->getRealAttribs()).push_back(CreateStringExpr("parallel", Context, KindLoc));
  if (Opt & IntelPragmaParallelAlways) {
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("QUAL_ALWAYS", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getRealAttribs()).push_back(CreateStringExpr(" always", Context, KindLoc));
  }
  if (Opt & IntelPragmaParallelAssert) {
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("QUAL_ASSERT", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getRealAttribs()).push_back(CreateStringExpr(" assert", Context, KindLoc));
  }
  if (Opt & IntelPragmaParallelCollapse) {
    Expr *CollapseExpr = Collapse.get();
    llvm::APSInt Res;
    if (!CollapseExpr->isIntegerConstantExpr(Res, Context)) {
      Diag(CollapseExpr->getLocStart(), diag::err_expr_not_ice) << 0;
      DeletePragmaOnError(stmt);
      return (StmtError());
    }
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("QUAL_COLLAPSE", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CollapseExpr, IntelPragmaExprRValue));
    (stmt->getRealAttribs()).push_back(CreateStringExpr(" collapse(", Context, KindLoc));
    (stmt->getRealAttribs()).push_back(CollapseExpr);
    (stmt->getRealAttribs()).push_back(CreateStringExpr(")", Context, KindLoc));
  }
  if (Opt & IntelPragmaParallelNumThreads) {
    Expr *NumThreadsExpr = NumThreads.get();
    llvm::APSInt Res;
    if (!NumThreadsExpr->isIntegerConstantExpr(Res, Context)) {
      Diag(NumThreadsExpr->getLocStart(), diag::err_expr_not_ice) << 0;
      DeletePragmaOnError(stmt);
      return (StmtError());
    }
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("QUAL_NUM_THREADS", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(NumThreadsExpr, IntelPragmaExprRValue));
    (stmt->getRealAttribs()).push_back(CreateStringExpr(" num_threads(", Context, KindLoc));
    (stmt->getRealAttribs()).push_back(NumThreadsExpr);
    (stmt->getRealAttribs()).push_back(CreateStringExpr(")", Context, KindLoc));
  }
  if (Opt & IntelPragmaParallelVars) {
    if (!Private.empty()) {
      CheckAndGenVars(S, KindLoc, Private, SizePrivate, " private", "QUAL_PRIVATE", stmt, SmallVector<ExprResult, 4>());
    }
    if (!LastPrivate.empty()) {
      // Commented because it is commented in icc compiler
      //CheckAndGenVars(S, KindLoc, LastPrivate, SizeLastPrivate, " lastprivate", "QUAL_LASTPRIVATE", stmt, Private);
      CheckAndGenVars(S, KindLoc, LastPrivate, SizeLastPrivate, " lastprivate", "QUAL_LASTPRIVATE", stmt, SmallVector<ExprResult, 4>());
    }
    if (!FirstPrivate.empty()) {
      CheckAndGenVars(S, KindLoc, FirstPrivate, SizeFirstPrivate, " firstprivate", "QUAL_FIRSTPRIVATE", stmt, Private);
    }
  }
  stmt->setPragmaKind(IntelPragmaParallel);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

// #pragma alloc_section
StmtResult Sema::ActOnPragmaOptionsAllocSection(SourceLocation KindLoc, const SmallVector<ExprResult, 4> &VarNames, const ExprResult &Section) {
  PragmaStmt* stmt = new (Context) PragmaStmt(KindLoc);
  Expr *SectExpr = Section.get();
  if (VarNames.size() > 0) {
    (stmt->getRealAttribs()).push_back(CreateStringExpr("alloc_section (", Context, KindLoc));
    Expr *Comma = CreateStringExpr(", ", Context, KindLoc);
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("ALLOC_SECTION", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(SectExpr, IntelPragmaExprConst));
    for (size_t i = 0; i < VarNames.size(); ++i) {
      if (!VarNames[i].isUsable()) continue;
      Expr *DeclExpr = VarNames[i].get()->IgnoreParenCasts();

      if (!isa<DeclRefExpr>(DeclExpr)) {
        Diag(DeclExpr->getLocStart(), diag::x_warn_intel_pragma_use_undecl)
           << DeclExpr->getSourceRange();
        continue;
      }
      ValueDecl *VD = cast<DeclRefExpr>(DeclExpr)->getDecl();
      if (!VD) continue;
      if (!isa<VarDecl>(VD)) {
        Diag(DeclExpr->getLocStart(), diag::x_warn_intel_pragma_use_undecl)
           << DeclExpr->getSourceRange();
        continue;
      }
      VarDecl *VrD = cast<VarDecl>(VD);
      if (!VrD) continue;
      if (!VrD->hasGlobalStorage()) {
        Diag(DeclExpr->getLocStart(), diag::x_warn_intel_pragma_var_local)
           << VrD->getIdentifier() << DeclExpr->getSourceRange();
        continue;
      }
      if (!VrD->isExternC()) {
        Diag(DeclExpr->getLocStart(), diag::x_warn_intel_pragma_not_C) << 0
           << VrD->getIdentifier() << DeclExpr->getSourceRange();
        continue;
      }
      // Mark all declarations referenced
      //MarkDeclRefReferenced(cast<DeclRefExpr>(DeclExpr));
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(DeclExpr, IntelPragmaExprLValue));
      (stmt->getRealAttribs()).push_back(DeclExpr);
      (stmt->getRealAttribs()).push_back(Comma);
    }
    if ((stmt->getRealAttribs()).size() > 1) {
      Expr *Quote = CreateStringExpr("\"", Context, KindLoc);
      (stmt->getRealAttribs()).push_back(Quote);
      (stmt->getRealAttribs()).push_back(SectExpr);
      (stmt->getRealAttribs()).push_back(Quote);
      (stmt->getRealAttribs()).push_back(CreateStringExpr(")", Context, KindLoc));
      stmt->setPragmaKind(IntelPragmaAllocSection);
    } 
    else {
      DeletePragmaOnError(stmt);
      return (StmtEmpty());
    }
  }
  else {
    DeletePragmaOnError(stmt);
    return (StmtEmpty());
  }
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

void Sema::ActOnPragmaOptionsAllocSection(PragmaStmt *Pragma) {
  //PragmaDecl *decl = PragmaDecl::Create(Context, Context.getTranslationUnitDecl(), Pragma->getSemiLoc());
  PragmaDecl *decl = PragmaDecl::Create(Context, CurContext, Pragma->getSemiLoc());
  
  MarkDeclarationsReferencedInPragma(*this, Pragma);
  Pragma->setDecl();
  decl->setStmt(Pragma);
  
  //Context.getTranslationUnitDecl()->addDecl(decl);
  CurContext->addDecl(decl);
  Consumer.HandleTopLevelDecl(DeclGroupRef(decl));
}

// #pragma section
enum SectionKind {
  AtRead, AtWrite, AtExecute, AtShared, AtNopage, AtNocache, AtDiscard, AtRemove, AtLong, AtShort, SectionKindSize
};

static bool IsMicrosoftStandardSectionName(const StringRef &Section) {
  return (Section == ".data" || Section == ".sdata" || Section == ".rdata" || Section == ".text" || 
    Section == ".bss" || Section == ".srdata" || Section == ".sbss");
}

StmtResult Sema::ActOnPragmaOptionsSection(SourceLocation KindLoc, const ExprResult &Section, const SmallVector<Token, 4> &AttrNames) {
  const char *Sections[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
  int SectionsTokens[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
  PragmaStmt* stmt = new (Context) PragmaStmt(KindLoc);
  Expr *SectExpr = Section.get();
  Expr *Comma = CreateStringExpr(", ", Context, KindLoc);
  Expr *Quote = CreateStringExpr("\"", Context, KindLoc);
  (stmt->getRealAttribs()).push_back(CreateStringExpr("section (", Context, KindLoc));
  (stmt->getRealAttribs()).push_back(Quote);
  (stmt->getRealAttribs()).push_back(SectExpr);
  (stmt->getRealAttribs()).push_back(Quote);
  (stmt->getRealAttribs()).push_back(Comma);
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("SECTION", Context, KindLoc), IntelPragmaExprConst));
  (stmt->getAttribs()).push_back(IntelPragmaAttrib(SectExpr, IntelPragmaExprConst));

  // SectExpr is always StringLiteral
  StringRef SectName = cast<StringLiteral>(SectExpr)->getString();
  bool IsMSSectionName = IsMicrosoftStandardSectionName(SectName);

  for (size_t i = 0; i < AttrNames.size(); ++i) {
    const IdentifierInfo *II = AttrNames[i].getIdentifierInfo();
    if (!II) continue;
    if (!II->isStr("read") && !II->isStr("write") && !II->isStr("execute") &&
      !II->isStr("shared") && !II->isStr("nopage") && !II->isStr("nocache") &&
      !II->isStr("discard") && !II->isStr("remove") && !II->isStr("long") && !II->isStr("short")) {
      Diag(AttrNames[i].getLocation(), diag::x_warn_intel_pragma_identifier_undefined)
         << II << AttrNames[i].getLocation();
      continue;
    }
    else if (II->isStr("short")) {
      Sections[AtShort] = "short";
      SectionsTokens[AtShort] = i;
      if (Sections[AtLong] != NULL) {
        Diag(AttrNames[i].getLocation(), diag::x_error_intel_pragma_conflicting_section_attributes);
        return (StmtError());
      }
    }
    else if (II->isStr("long")) {
      Sections[AtLong] = "long";
      SectionsTokens[AtLong] = i;
      if (Sections[AtShort] != NULL) {
        Diag(AttrNames[i].getLocation(), diag::x_error_intel_pragma_conflicting_section_attributes);
        return (StmtError());
      }
    }
    else if (II->isStr("read")) {
      Sections[AtRead] = "read";
      SectionsTokens[AtRead] = i;
    }
    else if (II->isStr("write")) {
      Sections[AtWrite] = "write";
      SectionsTokens[AtWrite] = i;
    }
    else if (II->isStr("execute")) {
      Sections[AtExecute] = "execute";
      SectionsTokens[AtExecute] = i;
    }
    else if (II->isStr("shared")) {
      Sections[AtShared] = "shared";
      SectionsTokens[AtShared] = i;
    }
    else if (II->isStr("nopage")) {
      Sections[AtNopage] = "nopage";
      SectionsTokens[AtNopage] = i;
    }
    else if (II->isStr("nocache")) {
      Sections[AtNocache] = "nocache";
      SectionsTokens[AtNocache] = i;
    }
    else if (II->isStr("discard")) {
      Sections[AtDiscard] = "discard";
      SectionsTokens[AtDiscard] = i;
    }
    else if (II->isStr("remove")) {
      Sections[AtRemove] = "remove";
      SectionsTokens[AtRemove] = i;
    }
  }
  if (IsMSSectionName) {
    Diag(KindLoc, diag::x_warn_intel_pragma_ignore_attributes_standard_section) << SectName;
    if (SectName == ".srdata") {
      Sections[AtRead] = "read";
      Sections[AtShort] = "short";
    }
  }
  else if (AttrNames.empty()) {
    Sections[AtRead] = "read";
    Sections[AtWrite] = "write";
  }
  for (size_t i = 0; i < SectionKindSize; ++i) {
    if (Sections[i] != NULL) {
      Expr *AttrExpr = CreateStringExpr(Sections[i], Context, 
        (SectionsTokens[i] != -1) ? AttrNames[SectionsTokens[i]].getLocation() : SourceLocation());
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(AttrExpr, IntelPragmaExprConst));
      (stmt->getRealAttribs()).push_back(AttrExpr);
      (stmt->getRealAttribs()).push_back(Comma);
    }
  }
  (stmt->getRealAttribs()).back() = CreateStringExpr(")", Context, KindLoc);
  stmt->setPragmaKind(IntelPragmaSection);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

void Sema::ActOnPragmaOptionsSection(PragmaStmt *Pragma) {
  //PragmaDecl *decl = PragmaDecl::Create(Context, Context.getTranslationUnitDecl(), Pragma->getSemiLoc());
  MarkDeclarationsReferencedInPragma(*this, Pragma);
  PragmaDecl *decl = PragmaDecl::Create(Context, CurContext, Pragma->getSemiLoc());
  
  Pragma->setDecl();
  decl->setStmt(Pragma);
  
  //Context.getTranslationUnitDecl()->addDecl(decl);
  CurContext->addDecl(decl);
  Consumer.HandleTopLevelDecl(DeclGroupRef(decl));
}

// #pragma alloc_text
#include "llvm/ADT/SmallSet.h"
static llvm::SmallSet<const FunctionDecl*, 10> FunctionsWithAllocText;
StmtResult Sema::ActOnPragmaOptionsAllocText(SourceLocation KindLoc, const ExprResult &Section, const SmallVector<ExprResult, 4> &FuncNames) {
  PragmaStmt* stmt = new (Context) PragmaStmt(KindLoc);
  Expr *SectExpr = Section.get();
  if (FuncNames.size() > 0) {
    (stmt->getRealAttribs()).push_back(CreateStringExpr("alloc_text (", Context, KindLoc));
    Expr *Comma = CreateStringExpr(", ", Context, KindLoc);
    for (size_t i = 0; i < FuncNames.size(); ++i) {
      if (!FuncNames[i].isUsable()) continue;
      Expr *DeclExpr = FuncNames[i].get()->IgnoreParenCasts();

      if (!isa<DeclRefExpr>(DeclExpr)) {
        Diag(DeclExpr->getLocStart(), diag::x_warn_intel_pragma_func_expect)
           << DeclExpr->getSourceRange();
        continue;
      }
      ValueDecl *VD = cast<DeclRefExpr>(DeclExpr)->getDecl();
      if (!VD) continue;
      if (!isa<FunctionDecl>(VD)) {
        Diag(DeclExpr->getLocStart(), diag::x_warn_intel_pragma_func_expect)
           << DeclExpr->getSourceRange();
        continue;
      }
      FunctionDecl *FD = cast<FunctionDecl>(VD);
      if (!FD) continue;
      if (!FD->isGlobal()) {
        Diag(DeclExpr->getLocStart(), diag::x_warn_intel_pragma_func_expect)
           << FD->getIdentifier() << DeclExpr->getSourceRange();
        continue;
      }
      if (!FD->isExternC()) {
        Diag(DeclExpr->getLocStart(), diag::x_warn_intel_pragma_not_C) << 1 
           << FD->getIdentifier() << DeclExpr->getSourceRange();
        continue;
      }
      if(FunctionsWithAllocText.insert(FD).second) {
        StringRef SectName = cast<StringLiteral>(SectExpr)->getString();
        FD->addAttr(::new (Context) SectionAttr(SourceRange(), Context, 
          CreateStringRef((std::string("__TEXT, ") + SectName.data()).c_str()), 0));
        (stmt->getRealAttribs()).push_back(DeclExpr);
        (stmt->getRealAttribs()).push_back(Comma);
      }
      else {
        Diag(DeclExpr->getLocStart(), diag::x_warn_intel_pragma_func_alloc_text)
           << DeclExpr->getSourceRange();
        continue;
      }
    }
    if ((stmt->getRealAttribs()).size() > 1) {
      Expr *Quote = CreateStringExpr("\"", Context, KindLoc);
      (stmt->getRealAttribs()).push_back(Quote);
      (stmt->getRealAttribs()).push_back(SectExpr);
      (stmt->getRealAttribs()).push_back(Quote);
      (stmt->getRealAttribs()).push_back(CreateStringExpr(")", Context, KindLoc));
      stmt->setPragmaKind(IntelPragmaAllocText);
    } 
    else {
      DeletePragmaOnError(stmt);
      return (StmtEmpty());
    }
  }
  else {
    DeletePragmaOnError(stmt);
    return (StmtEmpty());
  }
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

void Sema::ActOnPragmaOptionsAllocText(PragmaStmt *Pragma) {
  //PragmaDecl *decl = PragmaDecl::Create(Context, Context.getTranslationUnitDecl(), Pragma->getSemiLoc());
  PragmaDecl *decl = PragmaDecl::Create(Context, CurContext, Pragma->getSemiLoc());

  Pragma->setDecl();
  decl->setStmt(Pragma);

  //Context.getTranslationUnitDecl()->addDecl(decl);
  CurContext->addDecl(decl);
  Consumer.HandleTopLevelDecl(DeclGroupRef(decl));
}

// #pragma auto_inline
StmtResult Sema::ActOnPragmaOptionsAutoInline(SourceLocation KindLoc, 
  IntelPragmaAutoInlineOption Kind) {
  PragmaStmt *stmt = new (Context) PragmaStmt(KindLoc);

  switch (Kind) {
    case (IntelPragmaAutoInlineOptionOff):
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("NOAUTO_INLINE", Context, KindLoc), IntelPragmaExprConst));
      (stmt->getRealAttribs()).push_back(CreateStringExpr("auto_inline(off)", Context, KindLoc));
      break;
    default:
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("AUTO_INLINE", Context, KindLoc), IntelPragmaExprConst));
      (stmt->getRealAttribs()).push_back(CreateStringExpr("auto_inline(on)", Context, KindLoc));
      break;
  }

  stmt->setPragmaKind(IntelPragmaAutoInline);
  if (CommonFunctionOptions.count("AUTO_INLINE") > 0) {
    OptionsList[CommonFunctionOptions["AUTO_INLINE"]] = NULL;
  }
  CommonFunctionOptions["AUTO_INLINE"] = OptionsList.size();
  OptionsList.push_back(stmt);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

void Sema::ActOnPragmaOptionsAutoInline(PragmaStmt *Pragma) {
  //PragmaDecl *decl = PragmaDecl::Create(Context, Context.getTranslationUnitDecl(), Pragma->getSemiLoc());
  PragmaDecl *decl = PragmaDecl::Create(Context, CurContext, Pragma->getSemiLoc());

  Pragma->setDecl();
  decl->setStmt(Pragma);

  //Context.getTranslationUnitDecl()->addDecl(decl);
  CurContext->addDecl(decl);
  //Consumer.HandleTopLevelDecl(DeclGroupRef(decl));
  if (CommonFunctionOptions.count("AUTO_INLINE") > 0) {
    OptionsList[CommonFunctionOptions["AUTO_INLINE"]] = NULL;
  }
  CommonFunctionOptions["AUTO_INLINE"] = OptionsList.size();
  OptionsList.push_back(Pragma);
}

// #pragma bss_seg|code_seg|const_seg|data_seg
struct SegInfo {
  std::string Identifier;
  std::string SegName;
  std::string ClassName;
  SegInfo(const std::string &Id, const std::string &SN, const std::string &CN) :
    Identifier(Id), SegName(SN), ClassName(CN) {}
};
StmtResult Sema::ActOnPragmaOptionsSeg(SourceLocation KindLoc, 
  IntelPragmaSegKind Kind, IntelPragmaSegOption Opt, bool IdentifierSet, const std::string &Identifier,
  bool SegNameSet, const std::string &SegName, bool ClassNameSet, const std::string &ClassName) {

  static SmallVector<SegInfo, 4> BssStack, CodeStack, ConstStack, DataStack;
  SmallVector<SegInfo, 4> *CurrentStack;
  std::string realInfo;
  std::string RealSegName, RealClassName;
  bool StackEmpty = false;

  PragmaStmt *stmt = new (Context) PragmaStmt(KindLoc);

  switch (Kind) {
    case (IntelPragmaBssSeg):
      //(stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("BSS_SEG", Context, KindLoc), IntelPragmaExprConst));
      realInfo = "bss_seg";
      CurrentStack = &BssStack;
      break;
    case (IntelPragmaCodeSeg):
      //(stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("CODE_SEG", Context, KindLoc), IntelPragmaExprConst));
      realInfo = "code_seg";
      CurrentStack = &CodeStack;
      break;
    case (IntelPragmaConstSeg):
      //(stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("CONST_SEG", Context, KindLoc), IntelPragmaExprConst));
      realInfo = "const_seg";
      CurrentStack = &ConstStack;
      break;
    default:
      //(stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("DATA_SEG", Context, KindLoc), IntelPragmaExprConst));
      realInfo = "data_seg";
      CurrentStack = &DataStack;
      break;
  }
  if ((Opt != IntelPragmaSegOptionSet) || IdentifierSet || SegNameSet || ClassNameSet) {
    realInfo += "(";
    switch (Opt) {
      case (IntelPragmaSegOptionPush):
        realInfo += "push";
        CurrentStack->push_back(SegInfo(Identifier, SegName, ClassName));
        RealSegName = SegName;
        RealClassName = ClassName;
        break;
      case (IntelPragmaSegOptionPop):
        realInfo += "pop";
        if (IdentifierSet) {
          unsigned i;
          for (i = CurrentStack->size(); i > 0; --i) {
            if ((*CurrentStack)[i - 1].Identifier == Identifier) {
              for (unsigned j = CurrentStack->size(); j >= i; --j) {
                CurrentStack->pop_back();
              }
              if (!CurrentStack->empty()) {
                RealSegName = CurrentStack->back().SegName;
                RealClassName = CurrentStack->back().ClassName;
              }
              else {
                StackEmpty = true;
              }
              break;
            }
          }
          if (i == 0) {
            return (StmtEmpty());
          }
        }
        else if (!CurrentStack->empty()) {
          CurrentStack->pop_back();
          if (!CurrentStack->empty()) {
            RealSegName = CurrentStack->back().SegName;
            RealClassName = CurrentStack->back().ClassName;
          }
          else {
            StackEmpty = true;
          }
        }
        else {
          StackEmpty = true;
        }
        break;
      default:
        RealSegName = SegName;
        RealClassName = ClassName;
        break;
    }

    if (!StackEmpty) {
      if (RealSegName[0] == '\"') {
        RealSegName.erase(0, 1);
      }
      if (RealSegName[RealSegName.length() - 1] == '\"') {
        RealSegName.erase(RealSegName.length() - 1, 1);
      }
      if (RealClassName[0] == '\"') {
        RealClassName.erase(0, 1);
      }
      if (RealClassName[RealClassName.length() - 1] == '\"') {
        RealClassName.erase(RealClassName.length() - 1, 1);
      }

      if (Context.getTargetInfo().getTriple().isOSDarwin()) {
        switch (Kind) {
          case (IntelPragmaCodeSeg):
            RealSegName = "__TEXT, " + RealSegName;
            break;
          default:
            RealSegName = "__DATA, " + RealSegName;
            break;
        }
      }
      //(stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr(RealSegName.data(), Context, KindLoc), IntelPragmaExprConst));
      //(stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr(RealClassName.data(), Context, KindLoc), IntelPragmaExprConst));
      SegNames[Kind] = RealSegName;
      SegClasses[Kind] = RealClassName;
    }
    else {
      SegNames[Kind] = "";
      SegClasses[Kind] = "";
    }
    if (IdentifierSet) {
      realInfo += ", " + Identifier;
    }
    if (SegNameSet) {
      if (Opt != IntelPragmaSegOptionSet) {
        realInfo += ", ";
      }
      realInfo += SegName;
      if (ClassNameSet) {
        realInfo += ", " + ClassName;
      }
    }
    realInfo += ")";
  }
  (stmt->getRealAttribs()).push_back(CreateStringExpr(realInfo.data(), Context, KindLoc));

  stmt->setPragmaKind(IntelPragmaBCCDSeg);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

void Sema::ActOnPragmaOptionsSeg(PragmaStmt *Pragma) {
  //PragmaDecl *decl = PragmaDecl::Create(Context, Context.getTranslationUnitDecl(), Pragma->getSemiLoc());
  PragmaDecl *decl = PragmaDecl::Create(Context, CurContext, Pragma->getSemiLoc());
  
  MarkDeclarationsReferencedInPragma(*this, Pragma);
  Pragma->setDecl();
  decl->setStmt(Pragma);
  
  //Context.getTranslationUnitDecl()->addDecl(decl);
  CurContext->addDecl(decl);
  Consumer.HandleTopLevelDecl(DeclGroupRef(decl));
}

void Sema::ActOnVarFunctionDeclForSections(Decl *VFD) {
  if (VFD && isa<VarDecl>(VFD) && cast<VarDecl>(VFD)->hasGlobalStorage()) {
    VarDecl *VD = cast<VarDecl>(VFD);
    bool SectionIsSet = false;
    for (Decl::attr_iterator iter = VD->attr_begin(); iter != VD->attr_end(); ++iter) {
      if (isa<SectionAttr>(*iter)) {
        SectionIsSet = true;
        break;
      }
    }
    if (!SectionIsSet && VD->isThisDeclarationADefinition(Context) != VarDecl::DeclarationOnly) {
      if (VD->getType().isConstQualified()) {
        if (!SegNames[IntelPragmaConstSeg].empty()) {
          VD->addAttr(::new (Context) SectionAttr(SourceRange(), Context, 
            CreateStringRef(("#const#" + SegNames[IntelPragmaConstSeg] + "~@~" + SegClasses[IntelPragmaConstSeg]).c_str()), 0));
        }
      }
      else if (!VD->hasInit()) {
        if (!SegNames[IntelPragmaBssSeg].empty()) {
          VD->addAttr(::new (Context) SectionAttr(SourceRange(), Context, 
            CreateStringRef(("#bss#" + SegNames[IntelPragmaBssSeg] + "~@~" + SegClasses[IntelPragmaBssSeg]).c_str()), 0));
        }
      }
      else if (!SegNames[IntelPragmaDataSeg].empty()) {
        // Store all sectors to decide later which segment should be used
        VD->addAttr(::new (Context) SectionAttr(SourceRange(), Context, 
//          StringRef("#data#" + SegNames[IntelPragmaDataSeg] + "~@~" + SegClasses[IntelPragmaDataSeg] + 
//            "#bss#" + SegNames[IntelPragmaBssSeg] + "~@~" + SegClasses[IntelPragmaBssSeg])));
          CreateStringRef(("#data#" + SegNames[IntelPragmaDataSeg] + "~@~" + SegClasses[IntelPragmaDataSeg]).c_str()), 0));
      }
    }
  }
  else if (VFD && isa<FunctionDecl>(VFD) && cast<FunctionDecl>(VFD)->isThisDeclarationADefinition()) {
    FunctionDecl *FD = cast<FunctionDecl>(VFD);
    bool SectionIsSet = false;
    for (Decl::attr_iterator iter = FD->attr_begin(); iter != FD->attr_end(); ++iter) {
      if (isa<SectionAttr>(*iter)) {
        SectionIsSet = true;
        break;
      }
    }
    if (!SectionIsSet && !SegNames[IntelPragmaCodeSeg].empty()) {
      FD->addAttr(::new (Context) SectionAttr(SourceRange(), Context, 
        CreateStringRef(("#code#" + SegNames[IntelPragmaCodeSeg] + "~@~" + SegClasses[IntelPragmaCodeSeg]).c_str()), 0));
    }
  }
}

// #pragma check_stack
StmtResult Sema::ActOnPragmaOptionsCheckStack(SourceLocation KindLoc, 
  IntelPragmaCheckStackOption Kind) {
  PragmaStmt *stmt = new (Context) PragmaStmt(KindLoc);

  switch (Kind) {
    case (IntelPragmaCheckStackOptionOff):
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("NOCHECK_STACK", Context, KindLoc), IntelPragmaExprConst));
      (stmt->getRealAttribs()).push_back(CreateStringExpr("check_stack(off)", Context, KindLoc));
      break;
    default:
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("CHECK_STACK", Context, KindLoc), IntelPragmaExprConst));
      (stmt->getRealAttribs()).push_back(CreateStringExpr("check_stack(on)", Context, KindLoc));
      break;
  }

  stmt->setPragmaKind(IntelPragmaCheckStack);
  if (CommonFunctionOptions.count("CHECK_STACK") > 0) {
    OptionsList[CommonFunctionOptions["CHECK_STACK"]] = NULL;
  }
  CommonFunctionOptions["CHECK_STACK"] = OptionsList.size();
  OptionsList.push_back(stmt);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

void Sema::ActOnPragmaOptionsCheckStack(PragmaStmt *Pragma) {
  if (CommonFunctionOptions.count("CHECK_STACK") > 0) {
    OptionsList[CommonFunctionOptions["CHECK_STACK"]] = NULL;
  }
  CommonFunctionOptions["CHECK_STACK"] = OptionsList.size();
  OptionsList.push_back(Pragma);
  //PragmaDecl *decl = PragmaDecl::Create(Context, Context.getTranslationUnitDecl(), Pragma->getSemiLoc());
  PragmaDecl *decl = PragmaDecl::Create(Context, CurContext, Pragma->getSemiLoc());

  MarkDeclarationsReferencedInPragma(*this, Pragma);
  Pragma->setDecl();
  decl->setStmt(Pragma);

  //Context.getTranslationUnitDecl()->addDecl(decl);
  CurContext->addDecl(decl);
  //Consumer.HandleTopLevelDecl(DeclGroupRef(decl));
}

// #pragma init_seg
static bool InitSegSeen = false;
StmtResult Sema::ActOnPragmaOptionsInitSeg(SourceLocation KindLoc, const std::string &Section) {
  PragmaStmt *stmt = new (Context) PragmaStmt(KindLoc);

  if (InitSegSeen) {
    Diag(KindLoc, diag::x_warn_intel_pragma_init_seg_defined) << KindLoc;
    DeletePragmaOnError(stmt);
    return (StmtError());
  }

  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("INIT_SEG", Context, KindLoc), IntelPragmaExprConst));
  (stmt->getRealAttribs()).push_back(CreateStringExpr("init_seg(", Context, KindLoc));
  if (Section == "compiler") {
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr(".CRT$XCC", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getRealAttribs()).push_back(CreateStringExpr("compiler)", Context, KindLoc));
  }
  else if (Section == "lib") {
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr(".CRT$XCL", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getRealAttribs()).push_back(CreateStringExpr("lib)", Context, KindLoc));
  }
  else if (Section == "user") {
    (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr(".CRT$XCU", Context, KindLoc), IntelPragmaExprConst));
    (stmt->getRealAttribs()).push_back(CreateStringExpr("user)", Context, KindLoc));
  }
  else {
    (stmt->getRealAttribs()).push_back(CreateStringExpr(Section + ")", Context, KindLoc));
    std::string Sect = Section;
    if (Sect[0] == '\"') {
      Sect.erase(0, 1);
    }
    if (Sect[Sect.length() - 1] == '\"') {
      Sect.erase(Sect.length() - 1, 1);
    }
    if (Context.getTargetInfo().getTriple().isOSDarwin()) {
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("__DATA, " + Sect +  ", mod_init_funcs" , Context, KindLoc), 
        IntelPragmaExprConst));
    }
    else {
      DeletePragmaOnError(stmt);
      return (StmtEmpty());
    }
  }
  InitSegSeen = true;

  stmt->setPragmaKind(IntelPragmaInitSeg);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

void Sema::ActOnPragmaOptionsInitSeg(PragmaStmt *Pragma) {
  //PragmaDecl *decl = PragmaDecl::Create(Context, Context.getTranslationUnitDecl(), Pragma->getSemiLoc());
  PragmaDecl *decl = PragmaDecl::Create(Context, CurContext, Pragma->getSemiLoc());
  
  MarkDeclarationsReferencedInPragma(*this, Pragma);
  Pragma->setDecl();
  decl->setStmt(Pragma);
  
  //Context.getTranslationUnitDecl()->addDecl(decl);
  CurContext->addDecl(decl);
  Consumer.HandleTopLevelDecl(DeclGroupRef(decl));
}

// #pragma float_control

StmtResult Sema::ActOnPragmaCommonOnOff(SourceLocation KindLoc, const char *RealPragmaName,
  const char *SpecPragmaName, const IntelCommonDefaultOnOff DOO, const IntelPragmaCommonOnOff Kind, unsigned &FC) {
  PragmaStmt *stmt = new (Context) PragmaStmt(KindLoc);

  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr(SpecPragmaName, Context, KindLoc), IntelPragmaExprConst));
  switch (DOO) {
    case (IntelCommonOff):
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("OFF", Context, KindLoc), IntelPragmaExprConst));
      switch(Kind) {
        case (IntelPragmaFPContract):
          FC &= ~LangOptions::IFP_FP_Contract;
          break;
        case (IntelPragmaFEnvAccess):
          FC &= ~LangOptions::IFP_FEnv_Access;
          break;
      }
      break;
    case (IntelCommonOn):
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("ON", Context, KindLoc), IntelPragmaExprConst));
      switch(Kind) {
        case (IntelPragmaFPContract):
          FC |= LangOptions::IFP_FP_Contract;
          break;
        case (IntelPragmaFEnvAccess):
          if ((FC & LangOptions::IFP_Fast) || !(FC & LangOptions::IFP_ValueSafety)) {
            Diag(KindLoc, diag::x_error_intel_pragma_fenv_must_be_precise) << KindLoc;
            DeletePragmaOnError(stmt);
            return (StmtError());
          }
          FC |= LangOptions::IFP_FEnv_Access;
          break;
      }
      break;
    default:
      (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("DEFAULT", Context, KindLoc), IntelPragmaExprConst));
      break;
  }
  (stmt->getRealAttribs()).push_back(CreateStringExpr(RealPragmaName, Context, KindLoc));
  switch (DOO) {
    case (IntelCommonOff):
      (stmt->getRealAttribs()).push_back(CreateStringExpr("(off)", Context, KindLoc));
      break;
    case (IntelCommonOn):
      (stmt->getRealAttribs()).push_back(CreateStringExpr("(on)", Context, KindLoc));
      break;
    default:
      (stmt->getRealAttribs()).push_back(CreateStringExpr("(default)", Context, KindLoc));
      break;
  }
  stmt->setPragmaKind(IntelPragmaKindCommonOnOff);
  if (CommonFunctionOptions.count(SpecPragmaName) > 0) {
    OptionsList[CommonFunctionOptions[SpecPragmaName]] = NULL;
  }
  CommonFunctionOptions[SpecPragmaName] = OptionsList.size();
  OptionsList.push_back(stmt);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

void Sema::ActOnPragmaCommonOnOff(PragmaStmt *Pragma, const char *SpecPragmaName) {
  //PragmaDecl *decl = PragmaDecl::Create(Context, Context.getTranslationUnitDecl(), Pragma->getSemiLoc());
  PragmaDecl *decl = PragmaDecl::Create(Context, CurContext, Pragma->getSemiLoc());

  MarkDeclarationsReferencedInPragma(*this, Pragma);
  Pragma->setDecl();
  decl->setStmt(Pragma);

  //Context.getTranslationUnitDecl()->addDecl(decl);
  CurContext->addDecl(decl);
  if (CommonFunctionOptions.count(SpecPragmaName) > 0) {
    OptionsList[CommonFunctionOptions[SpecPragmaName]] = NULL;
  }
  CommonFunctionOptions[SpecPragmaName] = OptionsList.size();
  OptionsList.push_back(Pragma);
//  Consumer.HandleTopLevelDecl(DeclGroupRef(decl));
}

StmtResult Sema::ActOnPragmaOptionsFloatControl(SourceLocation KindLoc, unsigned &FC,
  IntelPragmaFloatControlOption Kind, IntelPragmaFloatControlOnOff OOS) {
  PragmaStmt *stmt = new (Context) PragmaStmt(KindLoc);
  StringRef FPModel;

  (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("FLOAT_CONTROL", Context, KindLoc), IntelPragmaExprConst));
  (stmt->getRealAttribs()).push_back(CreateStringExpr("float_control(", Context, KindLoc));
  switch (Kind) {
    case (IntelPragmaFloatControlExcept):
      switch (OOS) {
        case (IntelPragmaFloatControlOn):
          if ((FC & LangOptions::IFP_Fast) || !(FC & LangOptions::IFP_ValueSafety)) {
            Diag(KindLoc, diag::x_error_intel_pragma_must_be_precise) << KindLoc;
            DeletePragmaOnError(stmt);
            return (StmtError());
          }
          FC |= LangOptions::IFP_Except;
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("EXCEPT", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("ON", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getRealAttribs()).push_back(CreateStringExpr("except, on)", Context, KindLoc));
          break;
        default:
          //if (fenv_access is on) {additional action}
          FC &= ~LangOptions::IFP_Except;
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("EXCEPT", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("OFF", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getRealAttribs()).push_back(CreateStringExpr("except, off)", Context, KindLoc));
          break;
      }
      break;
    case (IntelPragmaFloatControlPrecise):
      switch (OOS) {
        case (IntelPragmaFloatControlOn):
          //*FC = IntelPragmaFloatControlPrecise | (*FC & IntelPragmaFloatControlExcept);
          FC = LangOptions::IFP_Precise|LangOptions::IFP_ValueSafety|
            (FC & (LangOptions::IFP_FP_Contract|LangOptions::IFP_Except|LangOptions::IFP_FEnv_Access));
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("PRECISE", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("ON", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getRealAttribs()).push_back(CreateStringExpr("precise, on)", Context, KindLoc));
          break;
        default:
          if (FC & LangOptions::IFP_Except) {
            Diag(KindLoc, diag::x_error_intel_pragma_except_off) << KindLoc;
            DeletePragmaOnError(stmt);
            return (StmtError());
          }
          else if (FC & LangOptions::IFP_FEnv_Access) {
            Diag(KindLoc, diag::x_error_intel_pragma_fenv_access_off) << KindLoc;
            DeletePragmaOnError(stmt);
            return (StmtError());
          }
          FC = LangOptions::IFP_Fast|LangOptions::IFP_Precise|LangOptions::IFP_Off|(FC & LangOptions::IFP_FP_Contract);
          //*FC = IntelPragmaFloatControlFast;
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("PRECISE", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("OFF", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getRealAttribs()).push_back(CreateStringExpr("precise, off)", Context, KindLoc));
          break;
      }
      break;
    case (IntelPragmaFloatControlSource):
      switch (OOS) {
        case (IntelPragmaFloatControlOn):
          //*FC = IntelPragmaFloatControlSource | (*FC & IntelPragmaFloatControlExcept);
          FC = LangOptions::IFP_Source|LangOptions::IFP_ValueSafety|
            (FC & (LangOptions::IFP_FP_Contract|LangOptions::IFP_Except|LangOptions::IFP_FEnv_Access));
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("SOURCE", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("ON", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getRealAttribs()).push_back(CreateStringExpr("source, on)", Context, KindLoc));
          break;
        default:
          if (FC & LangOptions::IFP_Except) {
            Diag(KindLoc, diag::x_error_intel_pragma_except_off) << KindLoc;
            DeletePragmaOnError(stmt);
            return (StmtError());
          }
          else if (FC & LangOptions::IFP_FEnv_Access) {
            Diag(KindLoc, diag::x_error_intel_pragma_fenv_access_off) << KindLoc;
            DeletePragmaOnError(stmt);
            return (StmtError());
          }
          //*FC = IntelPragmaFloatControlFast;
          FC = LangOptions::IFP_Fast|LangOptions::IFP_Source|LangOptions::IFP_Off|(FC & LangOptions::IFP_FP_Contract);
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("SOURCE", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("OFF", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getRealAttribs()).push_back(CreateStringExpr("source, off)", Context, KindLoc));
          break;
      }
      break;
    case (IntelPragmaFloatControlDouble):
      switch (OOS) {
        case (IntelPragmaFloatControlOn):
          //*FC = IntelPragmaFloatControlDouble | (*FC & IntelPragmaFloatControlExcept);
          FC = LangOptions::IFP_Double|LangOptions::IFP_ValueSafety|
            (FC & (LangOptions::IFP_FP_Contract|LangOptions::IFP_Except|LangOptions::IFP_FEnv_Access));
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("DOUBLE", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("ON", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getRealAttribs()).push_back(CreateStringExpr("double, on)", Context, KindLoc));
          break;
        default:
          if (FC & LangOptions::IFP_Except) {
            Diag(KindLoc, diag::x_error_intel_pragma_except_off) << KindLoc;
            DeletePragmaOnError(stmt);
            return (StmtError());
          }
          else if (FC & LangOptions::IFP_FEnv_Access) {
            Diag(KindLoc, diag::x_error_intel_pragma_fenv_access_off) << KindLoc;
            DeletePragmaOnError(stmt);
            return (StmtError());
          }
          //*FC = IntelPragmaFloatControlFast;
          FC = LangOptions::IFP_Fast|LangOptions::IFP_Double|LangOptions::IFP_Off|(FC & LangOptions::IFP_FP_Contract);
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("DOUBLE", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("OFF", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getRealAttribs()).push_back(CreateStringExpr("double, off)", Context, KindLoc));
          break;
      }
      break;
    case (IntelPragmaFloatControlExtended):
      switch (OOS) {
        case (IntelPragmaFloatControlOn):
          //*FC = IntelPragmaFloatControlExtended | (*FC & IntelPragmaFloatControlExcept);
          FC = LangOptions::IFP_Extended|LangOptions::IFP_ValueSafety|
            (FC & (LangOptions::IFP_FP_Contract|LangOptions::IFP_Except|LangOptions::IFP_FEnv_Access));
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("EXTENDED", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("ON", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getRealAttribs()).push_back(CreateStringExpr("extended, on)", Context, KindLoc));
          break;
        default:
          if (FC & LangOptions::IFP_Except) {
            Diag(KindLoc, diag::x_error_intel_pragma_except_off) << KindLoc;
            DeletePragmaOnError(stmt);
            return (StmtError());
          }
          else if (FC & LangOptions::IFP_FEnv_Access) {
            Diag(KindLoc, diag::x_error_intel_pragma_fenv_access_off) << KindLoc;
            DeletePragmaOnError(stmt);
            return (StmtError());
          }
          FC = LangOptions::IFP_Fast|LangOptions::IFP_Extended|LangOptions::IFP_Off|(FC & LangOptions::IFP_FP_Contract);
          //*FC = IntelPragmaFloatControlFast;
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("EXTENDED", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("OFF", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getRealAttribs()).push_back(CreateStringExpr("extended, off)", Context, KindLoc));
          break;
      }
      break;
    default: // Unknown float control - pop stack
      if (FC & LangOptions::IFP_Precise) {
        (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("PRECISE", Context, KindLoc), IntelPragmaExprConst));
        (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr((FC & LangOptions::IFP_Off) ? "OFF" : "ON", Context, KindLoc), IntelPragmaExprConst));
        (stmt->getRealAttribs()).push_back(CreateStringExpr((FC & LangOptions::IFP_Off) ? "precise, off)" : "precise, on)", Context, KindLoc));
      }
      else if (FC & LangOptions::IFP_Source) {
        (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("SOURCE", Context, KindLoc), IntelPragmaExprConst));
        (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr((FC & LangOptions::IFP_Off) ? "OFF" : "ON", Context, KindLoc), IntelPragmaExprConst));
        (stmt->getRealAttribs()).push_back(CreateStringExpr((FC & LangOptions::IFP_Off) ? "source, off)" : "source, on)", Context, KindLoc));
      }
      else if (FC & LangOptions::IFP_Double) {
        (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("DOUBLE", Context, KindLoc), IntelPragmaExprConst));
        (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr((FC & LangOptions::IFP_Off) ? "OFF" : "ON", Context, KindLoc), IntelPragmaExprConst));
        (stmt->getRealAttribs()).push_back(CreateStringExpr((FC & LangOptions::IFP_Off) ? "double, off)" : "double, on)", Context, KindLoc));
      }
      else if (FC & LangOptions::IFP_Extended) {
        (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("EXTENDED", Context, KindLoc), IntelPragmaExprConst));
        (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr((FC & LangOptions::IFP_Off) ? "OFF" : "ON", Context, KindLoc), IntelPragmaExprConst));
        (stmt->getRealAttribs()).push_back(CreateStringExpr((FC & LangOptions::IFP_Off) ? "extended, off)" : "extended, on)", Context, KindLoc));
      }
      if (FC & LangOptions::IFP_Except) {
        (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("EXCEPT", Context, KindLoc), IntelPragmaExprConst));
        (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("ON", Context, KindLoc), IntelPragmaExprConst));
        if ((stmt->getRealAttribs()).size() != 2)
          (stmt->getRealAttribs()).push_back(CreateStringExpr("except, on)", Context, KindLoc));
      }
      else if (!(FC & (LangOptions::IFP_Fast | LangOptions::IFP_Fast2))) {
        (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("EXCEPT", Context, KindLoc), IntelPragmaExprConst));
        (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("OFF", Context, KindLoc), IntelPragmaExprConst));
        if ((stmt->getRealAttribs()).size() != 2)
          (stmt->getRealAttribs()).push_back(CreateStringExpr("except, off)", Context, KindLoc));
      }
      else if ((stmt->getRealAttribs()).size() != 2) {
        (stmt->getRealAttribs()).push_back(CreateStringExpr("except, off)", Context, KindLoc));
        if (FC & LangOptions::IFP_Fast2) {
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("FAST2", Context, KindLoc), IntelPragmaExprConst));
          (stmt->getAttribs()).push_back(IntelPragmaAttrib(CreateStringExpr("ON", Context, KindLoc), IntelPragmaExprConst));
        }
      }
      break;
  }

  stmt->setPragmaKind(IntelPragmaFloatControl);
  if (CommonFunctionOptions.count("FLOAT_CONTROL") > 0) {
    OptionsList[CommonFunctionOptions["FLOAT_CONTROL"]] = NULL;
  }
  CommonFunctionOptions["FLOAT_CONTROL"] = OptionsList.size();
  OptionsList.push_back(stmt);
  MarkDeclarationsReferencedInPragma(*this, stmt);
  return StmtResult(stmt);
}

void Sema::ActOnPragmaOptionsFloatControl(Stmt *Pragma) {
  //PragmaDecl *decl = PragmaDecl::Create(Context, Context.getTranslationUnitDecl(), Pragma->getSemiLoc());
  PragmaStmt *stmt = cast<PragmaStmt>(Pragma);
  PragmaDecl *decl = PragmaDecl::Create(Context, CurContext, stmt->getSemiLoc());

  MarkDeclarationsReferencedInPragma(*this, stmt);
  stmt->setDecl();
  decl->setStmt(stmt);

  //Context.getTranslationUnitDecl()->addDecl(decl);
  CurContext->addDecl(decl);
  if (CommonFunctionOptions.count("FLOAT_CONTROL") > 0) {
    OptionsList[CommonFunctionOptions["FLOAT_CONTROL"]] = NULL;
  }
  CommonFunctionOptions["FLOAT_CONTROL"] = OptionsList.size();
  OptionsList.push_back(Pragma);
//    Consumer.HandleTopLevelDecl(DeclGroupRef(decl));
}

void Sema::DeletePragmaOnError(PragmaStmt *Pragma) {
  Pragma->turnToNullOp();
}

#endif  // INTEL_SPECIFIC_IL0_BACKEND
