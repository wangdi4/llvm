//===--- SemaStmtAttr.cpp - Statement Attribute Handling ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements stmt-related attribute processing.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/EvaluatedExprVisitor.h"
#include "clang/Sema/SemaInternal.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h" // INTEL
#include "clang/Sema/DelayedDiagnostic.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/ScopeInfo.h"
#include "llvm/ADT/StringExtras.h"

using namespace clang;
using namespace sema;

static Attr *handleFallThroughAttr(Sema &S, Stmt *St, const ParsedAttr &A,
                                   SourceRange Range) {
  FallThroughAttr Attr(S.Context, A);
  if (!isa<NullStmt>(St)) {
    S.Diag(A.getRange().getBegin(), diag::err_fallthrough_attr_wrong_target)
        << Attr.getSpelling() << St->getBeginLoc();
    if (isa<SwitchCase>(St)) {
      SourceLocation L = S.getLocForEndOfToken(Range.getEnd());
      S.Diag(L, diag::note_fallthrough_insert_semi_fixit)
          << FixItHint::CreateInsertion(L, ";");
    }
    return nullptr;
  }
  auto *FnScope = S.getCurFunction();
  if (FnScope->SwitchStack.empty()) {
    S.Diag(A.getRange().getBegin(), diag::err_fallthrough_attr_outside_switch);
    return nullptr;
  }

  // If this is spelled as the standard C++17 attribute, but not in C++17, warn
  // about using it as an extension.
  if (!S.getLangOpts().CPlusPlus17 && A.isCXX11Attribute() &&
      !A.getScopeName())
    S.Diag(A.getLoc(), diag::ext_cxx17_attr) << A;

  FnScope->setHasFallthroughStmt();
  return ::new (S.Context) FallThroughAttr(S.Context, A);
}

#if INTEL_CUSTOMIZATION
// Returns false if an invalid argument is detected
static bool HandleLoopFuseAttrArg(Sema &S, ArgsUnion AU,
                                  unsigned &DepthValue,
                                  bool &Independent) {
  if (AU.is<Expr *>()) {
    Expr *E = AU.get<Expr *>();
    if (!E)
      return true;

    llvm::APSInt ArgVal;
    if (E->isIntegerConstantExpr(ArgVal, S.getASTContext())) {
      if (!ArgVal.isStrictlyPositive()) {
        S.Diag(E->getExprLoc(), diag::err_attribute_requires_positive_integer)
            << "'loop_fuse'" << /* positive */ 0;
        return false;
      }
      DepthValue = ArgVal.getZExtValue();
      return true;
    }
    S.Diag(E->getExprLoc(), diag::err_loop_fuse_unknown_arg);
  } else if (AU.is<IdentifierLoc *>()) {
    IdentifierLoc *IE = AU.get<IdentifierLoc *>();
    if (!IE)
      return true;
    Independent = true;
    return true;
  }
  return true;
}

static Attr *handleLoopFuseAttr(Sema &S, Stmt *St, const ParsedAttr &A,
                                SourceRange Range) {
  unsigned NumArgs = A.getNumArgs();
  if (NumArgs > 2) {
    S.Diag(A.getLoc(), diag::err_attribute_too_many_arguments) << A << 2;
    return nullptr;
  }
  // Extract unsigned for depth and a bool for independent
  unsigned DepthValue = 0; // 0 stands for not-specified (default)
  bool Independent = false;
  for (unsigned i = 0; i < NumArgs; ++i)
    if (!HandleLoopFuseAttrArg(S, A.getArg(i), DepthValue, Independent))
      return nullptr;
  return ::new (S.Context) LoopFuseAttr(S.Context, A, DepthValue, Independent);
}
#endif // INTEL_CUSTOMIZATION

static Attr *handleSuppressAttr(Sema &S, Stmt *St, const ParsedAttr &A,
                                SourceRange Range) {
  if (A.getNumArgs() < 1) {
    S.Diag(A.getLoc(), diag::err_attribute_too_few_arguments) << A << 1;
    return nullptr;
  }

  std::vector<StringRef> DiagnosticIdentifiers;
  for (unsigned I = 0, E = A.getNumArgs(); I != E; ++I) {
    StringRef RuleName;

    if (!S.checkStringLiteralArgumentAttr(A, I, RuleName, nullptr))
      return nullptr;

    // FIXME: Warn if the rule name is unknown. This is tricky because only
    // clang-tidy knows about available rules.
    DiagnosticIdentifiers.push_back(RuleName);
  }

  return ::new (S.Context) SuppressAttr(
      S.Context, A, DiagnosticIdentifiers.data(), DiagnosticIdentifiers.size());
}

template <typename FPGALoopAttrT>
static Attr *handleIntelFPGALoopAttr(Sema &S, const ParsedAttr &A) {
  if(S.LangOpts.SYCLIsHost)
    return nullptr;

  unsigned NumArgs = A.getNumArgs();
#if INTEL_CUSTOMIZATION
  // Warn if this is not spelled as the pragma since pragma can take
  // five arguments and SYCL form can only take one argument.
  if ((NumArgs > 1) &&
      (A.getSyntax() != AttributeCommonInfo::AS_Pragma)) {
#endif // INTEL_CUSTOMIZATION
    S.Diag(A.getLoc(), diag::warn_attribute_too_many_arguments) << A << 1;
    return nullptr;
  }

  if (NumArgs == 0) {
    if (A.getKind() == ParsedAttr::AT_SYCLIntelFPGAII ||
        A.getKind() == ParsedAttr::AT_SYCLIntelFPGAMaxConcurrency ||
        A.getKind() == ParsedAttr::AT_SYCLIntelFPGAMaxInterleaving ||
        A.getKind() == ParsedAttr::AT_SYCLIntelFPGASpeculatedIterations) {
      S.Diag(A.getLoc(), diag::warn_attribute_too_few_arguments) << A << 1;
      return nullptr;
    }
  }

#if INTEL_CUSTOMIZATION
  if (A.getSyntax() == AttributeCommonInfo::AS_Pragma)
    return S.BuildSYCLIntelFPGALoopAttr<FPGALoopAttrT>(
        A, A.getArgAsExpr(3));
  else
#endif // INTEL_CUSTOMIZATION
  return S.BuildSYCLIntelFPGALoopAttr<FPGALoopAttrT>(
      A, A.getNumArgs() ? A.getArgAsExpr(0) : nullptr);
}

template <>
Attr *handleIntelFPGALoopAttr<SYCLIntelFPGADisableLoopPipeliningAttr>(
    Sema &S, const ParsedAttr &A) {
  if (S.LangOpts.SYCLIsHost)
    return nullptr;

  unsigned NumArgs = A.getNumArgs();
#if INTEL_CUSTOMIZATION
  // Warn if this is not spelled as the pragma since pragma can take
  // five arguments and SYCL form can take zero argument.
  if ((NumArgs > 0) &&
      (A.getSyntax() != AttributeCommonInfo::AS_Pragma)) {
#endif // INTEL_CUSTOMIZATION
    S.Diag(A.getLoc(), diag::warn_attribute_too_many_arguments) << A << 0;
    return nullptr;
  }

  return new (S.Context) SYCLIntelFPGADisableLoopPipeliningAttr(S.Context, A);
}

static bool checkSYCLIntelFPGAIVDepSafeLen(Sema &S, llvm::APSInt &Value,
                                           Expr *E) {
  if (!Value.isStrictlyPositive())
    return S.Diag(E->getExprLoc(),
                  diag::err_attribute_requires_positive_integer)
           << "'ivdep'" << /* positive */ 0;
  return false;
}

enum class IVDepExprResult {
  Invalid,
  Null,
  Dependent,
  Array,
  SafeLen,
};

static IVDepExprResult HandleFPGAIVDepAttrExpr(Sema &S, Expr *E,
                                               unsigned &SafelenValue) {
  if (!E)
    return IVDepExprResult::Null;

  if (E->isInstantiationDependent())
    return IVDepExprResult::Dependent;

  llvm::APSInt ArgVal;

  if (E->isIntegerConstantExpr(ArgVal, S.getASTContext())) {
    if (checkSYCLIntelFPGAIVDepSafeLen(S, ArgVal, E))
      return IVDepExprResult::Invalid;
    SafelenValue = ArgVal.getZExtValue();
    return IVDepExprResult::SafeLen;
  }

  if (isa<DeclRefExpr>(E) || isa<MemberExpr>(E)) {
    if (!E->getType()->isArrayType() && !E->getType()->isPointerType()) {
      S.Diag(E->getExprLoc(), diag::err_ivdep_declrefexpr_arg);
      return IVDepExprResult::Invalid;
    }
    return IVDepExprResult::Array;
  }

  S.Diag(E->getExprLoc(), diag::err_ivdep_unknown_arg);
  return IVDepExprResult::Invalid;
}

// Note: At the time of this call, we don't know the order of the expressions,
// so we name them vaguely until we can figure it out.
SYCLIntelFPGAIVDepAttr *
Sema::BuildSYCLIntelFPGAIVDepAttr(const AttributeCommonInfo &CI, Expr *Expr1,
                                  Expr *Expr2) {
  unsigned SafelenValue = 0;
  IVDepExprResult E1 = HandleFPGAIVDepAttrExpr(*this, Expr1, SafelenValue);
  IVDepExprResult E2 = HandleFPGAIVDepAttrExpr(*this, Expr2, SafelenValue);

  if (E1 == IVDepExprResult::Invalid || E2 == IVDepExprResult::Invalid)
    return nullptr;

  if (E1 == E2 && E1 != IVDepExprResult::Dependent &&
      E1 != IVDepExprResult::Null) {
    Diag(Expr2->getExprLoc(), diag::err_ivdep_duplicate_arg);
    return nullptr;
  }

  // Try to put Safelen in the 1st one so codegen can count on the ordering.
  Expr *SafeLenExpr;
  Expr *ArrayExpr;
  if (E1 == IVDepExprResult::SafeLen) {
    SafeLenExpr = Expr1;
    ArrayExpr = Expr2;
  } else {
    SafeLenExpr = Expr2;
    ArrayExpr = Expr1;
  }

  return new (Context)
      SYCLIntelFPGAIVDepAttr(Context, CI, SafeLenExpr, ArrayExpr, SafelenValue);
}

// Filters out any attributes from the list that are either not the specified
// type, or whose function isDependent returns true.
template <typename T>
static void FilterAttributeList(ArrayRef<const Attr *> Attrs,
                    SmallVectorImpl<const T *> &FilteredAttrs) {

  llvm::transform(Attrs, std::back_inserter(FilteredAttrs), [](const Attr *A) {
    if (const auto *Cast = dyn_cast_or_null<const T>(A))
      return Cast->isDependent() ? nullptr : Cast;
    return static_cast<const T*>(nullptr);
  });
  FilteredAttrs.erase(
      std::remove(FilteredAttrs.begin(), FilteredAttrs.end(),
                  static_cast<const T*>(nullptr)),
      FilteredAttrs.end());
}

static void
CheckRedundantSYCLIntelFPGAIVDepAttrs(Sema &S, ArrayRef<const Attr *> Attrs) {
  // Skip SEMA if we're in a template, this will be diagnosed later.
  if (S.getCurLexicalContext()->isDependentContext())
    return;

  SmallVector<const SYCLIntelFPGAIVDepAttr *, 8> FilteredAttrs;
  // Filter down to just non-dependent ivdeps.
  FilterAttributeList(Attrs, FilteredAttrs);
  if (FilteredAttrs.empty())
    return;

  SmallVector<const SYCLIntelFPGAIVDepAttr *, 8> SortedAttrs(FilteredAttrs);
  llvm::stable_sort(SortedAttrs, SYCLIntelFPGAIVDepAttr::SafelenCompare);

  // Find the maximum without an array expression, which ends up in the 2nd
  // expr.
  const auto *GlobalMaxItr =
      llvm::find_if(SortedAttrs, [](const SYCLIntelFPGAIVDepAttr *A) {
        return !A->getArrayExpr();
      });
  const SYCLIntelFPGAIVDepAttr *GlobalMax =
      GlobalMaxItr == SortedAttrs.end() ? nullptr : *GlobalMaxItr;

  for (const auto *A : FilteredAttrs) {
    if (A == GlobalMax)
      continue;

    if (GlobalMax && !SYCLIntelFPGAIVDepAttr::SafelenCompare(A, GlobalMax)) {
      S.Diag(A->getLocation(), diag::warn_ivdep_redundant)
          << !GlobalMax->isInf() << GlobalMax->getSafelenValue() << !A->isInf()
          << A->getSafelenValue();
      S.Diag(GlobalMax->getLocation(), diag::note_previous_attribute);
      continue;
    }

    if (!A->getArrayExpr())
      continue;

    const ValueDecl *ArrayDecl = A->getArrayDecl();
    auto Other = llvm::find_if(SortedAttrs,
                               [ArrayDecl](const SYCLIntelFPGAIVDepAttr *A) {
                                 return ArrayDecl == A->getArrayDecl();
                               });
    assert(Other != SortedAttrs.end() && "Should find at least itself");

    // Diagnose if lower/equal to the lowest with this array.
    if (*Other != A && !SYCLIntelFPGAIVDepAttr::SafelenCompare(A, *Other)) {
      S.Diag(A->getLocation(), diag::warn_ivdep_redundant)
          << !(*Other)->isInf() << (*Other)->getSafelenValue() << !A->isInf()
          << A->getSafelenValue();
      S.Diag((*Other)->getLocation(), diag::note_previous_attribute);
    }
  }
}

static Attr *handleIntelFPGAIVDepAttr(Sema &S, const ParsedAttr &A) {
  unsigned NumArgs = A.getNumArgs();
  if (NumArgs > 2) {
    S.Diag(A.getLoc(), diag::err_attribute_too_many_arguments) << A << 2;
    return nullptr;
  }

  return S.BuildSYCLIntelFPGAIVDepAttr(
      A, NumArgs >= 1 ? A.getArgAsExpr(0) : nullptr,
      NumArgs == 2 ? A.getArgAsExpr(1) : nullptr);
}

#if INTEL_CUSTOMIZATION
static Attr *handleHLSIVDepAttr(Sema &S, const ParsedAttr &A) {
  Expr* ValueExpr = A.getArgAsExpr(3);
  Expr *ArrayExpr = A.getArgAsExpr(4);

  if (ValueExpr == nullptr)
    std::swap(ValueExpr, ArrayExpr);

  return S.BuildSYCLIntelFPGAIVDepAttr(A, ValueExpr, ArrayExpr);
}
#endif // INTEL_CUSTOMIZATION

static Attr *handleLoopHintAttr(Sema &S, Stmt *St, const ParsedAttr &A,
                                SourceRange) {
  IdentifierLoc *PragmaNameLoc = A.getArgAsIdent(0);
  IdentifierLoc *OptionLoc = A.getArgAsIdent(1);
  IdentifierLoc *StateLoc = A.getArgAsIdent(2);
  Expr *ValueExpr = A.getArgAsExpr(3);
#if INTEL_CUSTOMIZATION
  Expr *ArrayExpr = A.getArgAsExpr(4);
#endif // INTEL_CUSTOMIZATION

  StringRef PragmaName =
      llvm::StringSwitch<StringRef>(PragmaNameLoc->Ident->getName())
          .Cases("unroll", "nounroll", "unroll_and_jam", "nounroll_and_jam",
                 PragmaNameLoc->Ident->getName())
#if INTEL_CUSTOMIZATION
          .Cases("ivdep", "ii_at_most", "ii_at_least", "min_ii_at_target_fmax",
                 PragmaNameLoc->Ident->getName())
          .Cases("force_hyperopt", "force_no_hyperopt", "nofusion", "fusion",
                 PragmaNameLoc->Ident->getName())
          .Cases("vector", "novector", "loop_count","distribute_point",
                 PragmaNameLoc->Ident->getName())
#endif // INTEL_CUSTOMIZATION
          .Default("clang loop");
#if INTEL_CUSTOMIZATION
  bool PragmaDistributePoint =
      PragmaNameLoc->Ident->getName() == "distribute_point";
  bool NonLoopPragmaDistributePoint =
      PragmaDistributePoint && St->getStmtClass() != Stmt::DoStmtClass &&
      St->getStmtClass() != Stmt::ForStmtClass &&
      St->getStmtClass() != Stmt::CXXForRangeStmtClass &&
      St->getStmtClass() != Stmt::WhileStmtClass;
  if (NonLoopPragmaDistributePoint) {
    bool withinLoop = false;
    for (Scope *CS = S.getCurScope(); CS; CS = CS->getParent())
      if (CS->getFlags() & Scope::ContinueScope) {
        withinLoop = true;
        break;
      }
    if (!withinLoop) {
      S.Diag(St->getBeginLoc(), diag::err_pragma_distpt_on_nonloop_stmt)
          << "#pragma distribute_point";
      return nullptr;
    }
  } else
#endif // INTEL_CUSTOMIZATION
  if (St->getStmtClass() != Stmt::DoStmtClass &&
      St->getStmtClass() != Stmt::ForStmtClass &&
      St->getStmtClass() != Stmt::CXXForRangeStmtClass &&
      St->getStmtClass() != Stmt::WhileStmtClass) {
    std::string Pragma = "#pragma " + std::string(PragmaName);
    S.Diag(St->getBeginLoc(), diag::err_pragma_loop_precedes_nonloop) << Pragma;
    return nullptr;
  }

  LoopHintAttr::OptionType Option;
  LoopHintAttr::LoopHintState State;

  auto SetHints = [&Option, &State](LoopHintAttr::OptionType O,
                                    LoopHintAttr::LoopHintState S) {
    Option = O;
    State = S;
  };
  if (PragmaName == "nounroll") {
    SetHints(LoopHintAttr::Unroll, LoopHintAttr::Disable);
  } else if (PragmaName == "unroll") {
    // #pragma unroll N
    if (ValueExpr) {
      // #pragma unroll N
      SetHints(LoopHintAttr::UnrollCount, LoopHintAttr::Numeric);
#if INTEL_CUSTOMIZATION
      // CQ#366562 - let #pragma unroll value be out of striclty positive 32-bit
      // integer range: disable unrolling if value is 0, otherwise treat this
      // like #pragma unroll without argument.
      // CQ#415958 - ignore this behavior in template types (isValueDependent).
      if (S.getLangOpts().IntelCompat && !ValueExpr->isValueDependent()) {
        llvm::APSInt Val;
        ExprResult Res = S.VerifyIntegerConstantExpression(ValueExpr, &Val);
        if (Res.isInvalid())
          return nullptr;

        bool ValueIsPositive = Val.isStrictlyPositive();
        if (!ValueIsPositive || Val.getActiveBits() > 31) {
          // Non-zero (negative or too large) value: just ignore the argument.
          // #pragma unroll(0) disables unrolling.
          State =
              Val.getBoolValue() ? LoopHintAttr::Enable : LoopHintAttr::Disable;
          SetHints(LoopHintAttr::Unroll, State);
        }
      }
#endif // INTEL_CUSTOMIZATION
    } else {
      // #pragma unroll
      SetHints(LoopHintAttr::Unroll, LoopHintAttr::Enable);
    }
  } else if (PragmaName == "nounroll_and_jam") {
    SetHints(LoopHintAttr::UnrollAndJam, LoopHintAttr::Disable);
  } else if (PragmaName == "unroll_and_jam") {
    // #pragma unroll_and_jam N
    if (ValueExpr)
      SetHints(LoopHintAttr::UnrollAndJamCount, LoopHintAttr::Numeric);
    else
      SetHints(LoopHintAttr::UnrollAndJam, LoopHintAttr::Enable);
#if INTEL_CUSTOMIZATION
  } else if (PragmaName == "ivdep") {
    bool HLSCompat =
          S.getLangOpts().HLS ||
          (S.getLangOpts().OpenCL &&
           S.Context.getTargetInfo().getTriple().isINTELFPGAEnvironment());
    bool IntelCompat = S.getLangOpts().IntelCompat;
    if (HLSCompat) {
      return handleHLSIVDepAttr(S, A);
    } /* if */
    if (ValueExpr && ArrayExpr) {
      SetHints(LoopHintAttr::IVDepHLS, LoopHintAttr::Full);
    } else if (ValueExpr) {
      SetHints(LoopHintAttr::IVDepHLS, LoopHintAttr::Numeric);
    } else if (ArrayExpr) {
      SetHints(LoopHintAttr::IVDepHLS, LoopHintAttr::LoopExpr);
    } else if (OptionLoc->Ident && OptionLoc->Ident->getName() == "loop") {
      SetHints(LoopHintAttr::IVDepLoop, LoopHintAttr::Enable);
    } else if (OptionLoc->Ident && OptionLoc->Ident->getName() == "back") {
      SetHints(LoopHintAttr::IVDepBack, LoopHintAttr::Enable);
    } else {
      if (HLSCompat && IntelCompat)
        SetHints(LoopHintAttr::IVDepHLSIntel, LoopHintAttr::Enable);
      else if (HLSCompat)
        SetHints(LoopHintAttr::IVDepHLS, LoopHintAttr::Enable);
      else
        SetHints(LoopHintAttr::IVDep, LoopHintAttr::Enable);
    }
  } else if (PragmaName == "ii_at_most") {
    SetHints(LoopHintAttr::IIAtMost, LoopHintAttr::Numeric);
  } else if (PragmaName == "ii_at_least") {
    SetHints(LoopHintAttr::IIAtLeast, LoopHintAttr::Numeric);
  } else if (PragmaName == "min_ii_at_target_fmax") {
    SetHints(LoopHintAttr::MinIIAtFmax, LoopHintAttr::Enable);
  } else if (PragmaName == "force_hyperopt") {
    SetHints(LoopHintAttr::ForceHyperopt, LoopHintAttr::Enable);
  } else if (PragmaName == "force_no_hyperopt") {
    SetHints(LoopHintAttr::ForceHyperopt, LoopHintAttr::Disable);
  } else if (PragmaName == "distribute_point") {
    SetHints(LoopHintAttr::Distribute, LoopHintAttr::Enable);
  } else if (PragmaName == "nofusion") {
    SetHints(LoopHintAttr::Fusion, LoopHintAttr::Disable);
  } else if (PragmaName == "fusion") {
    SetHints(LoopHintAttr::Fusion, LoopHintAttr::Enable);
  } else if (PragmaName == "novector") {
    SetHints(LoopHintAttr::Vectorize, LoopHintAttr::Disable);
  } else if (PragmaName == "vector") {
    assert(OptionLoc && OptionLoc->Ident &&
           "Attribute must have valid option info.");
    Option = llvm::StringSwitch<LoopHintAttr::OptionType>(
                 OptionLoc->Ident->getName())
                 .Case("always", LoopHintAttr::VectorizeAlways)
                 .Default(LoopHintAttr::Vectorize);
    SetHints(Option, LoopHintAttr::Enable);
  } else if (PragmaName == "loop_count") {
    assert(OptionLoc && OptionLoc->Ident &&
           "Attribute must have valid option info.");
    Option = llvm::StringSwitch<LoopHintAttr::OptionType>(
                 OptionLoc->Ident->getName())
                 .Case("loop_count", LoopHintAttr::LoopCount)
                 .Case("min", LoopHintAttr::LoopCountMin)
                 .Case("max", LoopHintAttr::LoopCountMax)
                 .Case("avg", LoopHintAttr::LoopCountAvg);
    SetHints(Option, LoopHintAttr::Numeric);
#endif // INTEL_CUSTOMIZATION
  } else {
    // #pragma clang loop ...
    assert(OptionLoc && OptionLoc->Ident &&
           "Attribute must have valid option info.");
    Option = llvm::StringSwitch<LoopHintAttr::OptionType>(
                 OptionLoc->Ident->getName())
                 .Case("vectorize", LoopHintAttr::Vectorize)
                 .Case("vectorize_width", LoopHintAttr::VectorizeWidth)
                 .Case("interleave", LoopHintAttr::Interleave)
                 .Case("vectorize_predicate", LoopHintAttr::VectorizePredicate)
                 .Case("interleave_count", LoopHintAttr::InterleaveCount)
                 .Case("unroll", LoopHintAttr::Unroll)
                 .Case("unroll_count", LoopHintAttr::UnrollCount)
                 .Case("pipeline", LoopHintAttr::PipelineDisabled)
                 .Case("pipeline_initiation_interval",
                       LoopHintAttr::PipelineInitiationInterval)
                 .Case("distribute", LoopHintAttr::Distribute)
                 .Default(LoopHintAttr::Vectorize);
    if (Option == LoopHintAttr::VectorizeWidth ||
        Option == LoopHintAttr::InterleaveCount ||
        Option == LoopHintAttr::UnrollCount ||
        Option == LoopHintAttr::PipelineInitiationInterval) {
      assert(ValueExpr && "Attribute must have a valid value expression.");
      if (S.CheckLoopHintExpr(ValueExpr, St->getBeginLoc()))
        return nullptr;
      State = LoopHintAttr::Numeric;
    } else if (Option == LoopHintAttr::Vectorize ||
               Option == LoopHintAttr::Interleave ||
               Option == LoopHintAttr::VectorizePredicate ||
               Option == LoopHintAttr::Unroll ||
               Option == LoopHintAttr::Distribute ||
               Option == LoopHintAttr::PipelineDisabled) {
      assert(StateLoc && StateLoc->Ident && "Loop hint must have an argument");
      if (StateLoc->Ident->isStr("disable"))
        State = LoopHintAttr::Disable;
      else if (StateLoc->Ident->isStr("assume_safety"))
        State = LoopHintAttr::AssumeSafety;
      else if (StateLoc->Ident->isStr("full"))
        State = LoopHintAttr::Full;
      else if (StateLoc->Ident->isStr("enable"))
        State = LoopHintAttr::Enable;
      else
        llvm_unreachable("bad loop hint argument");
    } else
      llvm_unreachable("bad loop hint");
  }

#if INTEL_CUSTOMIZATION
  return LoopHintAttr::CreateImplicit(S.Context, Option, State, ValueExpr,
                                      ArrayExpr, A);
#endif // INTEL_CUSTOMIZATION
}

#if INTEL_CUSTOMIZATION
static Attr *handleIntelInlineAttr(Sema &S, Stmt *St, const ParsedAttr &A,
                                   SourceRange) {
  IdentifierLoc *OptionLoc = A.getArgAsIdent(1);
  IntelInlineAttr::OptionType Option;
  if (OptionLoc) {
    if (OptionLoc->Ident->getName() != "recursive") {
      S.Diag(OptionLoc->Loc, diag::err_recursive_attribute_expected);
      return nullptr;
    }
    else {
      Option = IntelInlineAttr::Recursive;
    }
  }
  else {
    Option = IntelInlineAttr::NotRecursive;
  }
  return IntelInlineAttr::CreateImplicit(S.Context, Option, A);
}

static int64_t getConstInt(Sema &S, unsigned Idx, const ParsedAttr &A) {
  ASTContext &Ctx = S.getASTContext();
  Expr *E = A.getArgAsExpr(Idx);
  llvm::APSInt ValueAPS = (E)->EvaluateKnownConstInt(Ctx);
  if (ValueAPS.getActiveBits() > std::numeric_limits<int64_t>::digits)
    return std::numeric_limits<int64_t>::max();  //error will emit later.
  return ValueAPS.getSExtValue();
}

static bool diagOverlapingLevels(Sema &S, int LevelFrom, int LevelTo,
                                 SourceLocation SourceLoc,
                                 int PrevFrom, int PrevTo,
                                 SourceLocation PrevSourceLoc) {
  S.Diag(SourceLoc, diag::err_blocklevel_overlap)
      << LevelFrom << LevelTo << PrevFrom << PrevTo;
  return S.Diag(PrevSourceLoc,
                diag::note_second_block_loop_level_specified_here);
}

namespace {
struct PragmaBlockLoopLevelInfo {
  Expr *Factor;
  int LevelFrom;
  int LevelTo;
  clang::SourceLocation SourceLoc;
};
using MapType = llvm::SmallDenseMap<int, PragmaBlockLoopLevelInfo>;
}

static bool checkOverlapingLevels(Sema &S, Expr *FE, int64_t LevelFrom,
                                  int64_t LevelTo, SourceLocation SourceLoc,
                                  MapType &Map) {
  if (LevelFrom > LevelTo)
    return S.Diag(SourceLoc, diag::err_invalid_blocklevel);

  if (LevelFrom > 8 || LevelTo > 8)
    return S.Diag(SourceLoc, diag::err_invalid_blocklevel_range) << 1;

  if (LevelFrom <= 0 || LevelTo <= 0)
    return S.Diag(SourceLoc, diag::err_invalid_blocklevel_range) << 0;

  for (int L = LevelFrom; L <= LevelTo; ++L) {
    // If found level L or found level -1 diagnostic overlapping. Level -1
    // represent all levels.
    MapType::iterator It = Map.find(L);
    if (It == Map.end())
      It = Map.find(-1);
    if (It != Map.end())
      return diagOverlapingLevels(S, LevelFrom, LevelTo, SourceLoc,
                           It->second.LevelFrom, It->second.LevelTo,
                           It->second.SourceLoc);
    Map[L].Factor = FE;
    Map[L].LevelFrom = LevelFrom;
    Map[L].LevelTo = LevelTo;
    Map[L].SourceLoc = SourceLoc;
  }
  return false;
}

static Attr *handleIntelBlockLoopAttr(Sema &S, Stmt *St, const ParsedAttr &AA,
                                      const ParsedAttributesView &AttrList,
                                      SourceRange) {

  // If BlockLoop attribute is not the first one on the attribute list,
  // return, since all BlockLoop attributes have been processed during
  // processing first BlockLoop attribute.
  const auto &A = llvm::find_if(AttrList, [](const ParsedAttr &Attr) {
    return Attr.getKind() == ParsedAttr::AT_IntelBlockLoop;
  });
  if (&AA != &*A)
    return nullptr;

  if (St->getStmtClass() != Stmt::DoStmtClass &&
      St->getStmtClass() != Stmt::ForStmtClass &&
      St->getStmtClass() != Stmt::CXXForRangeStmtClass &&
      St->getStmtClass() != Stmt::WhileStmtClass) {
    SmallString<24> DiagStr("#pragma ");
    DiagStr += AA.getArgAsIdent(0)->Ident->getName();
    S.Diag(St->getBeginLoc(), diag::err_pragma_loop_precedes_nonloop)
        << DiagStr;
    return nullptr;
  }

  SmallVector<Expr *, 2> Privates;
  MapType Map;  //interal use only for disanostic levels overlapping.
  for (const ParsedAttr &A : AttrList)
    if (A.getKind() == ParsedAttr::AT_IntelBlockLoop) {
      unsigned AI = 0;
      Expr *FE = nullptr;
      clang::SourceLocation SourceLoc;
      SourceLoc = A.getArgAsIdent(AI++)->Loc;//beginning of block_loop pragma
      if (A.isArgIdent(AI) &&
          A.getArgAsIdent(AI)->Ident->isStr("factor")) {
        FE = A.getArgAsExpr(++AI);
        AI++;
      }
      if (A.isArgIdent(AI) &&
          A.getArgAsIdent(AI)->Ident->isStr("level")) {
        SourceLoc = A.getArgAsIdent(AI)->Loc; //beginning of Level clause
        int64_t LevelFrom = getConstInt(S, ++AI, A);
        int64_t LevelTo = getConstInt(S, ++AI, A);
        AI++;
        if (checkOverlapingLevels(S, FE, LevelFrom, LevelTo, SourceLoc, Map))
          return nullptr;
      } else {
        if (!Map.empty()) {
          // No user specified level for current pragma, but other block_loop
          // has user specified level, diagnostic overlapping.
          MapType::iterator It = Map.begin();
          diagOverlapingLevels(S, -1, -1, SourceLoc, It->second.LevelFrom,
                               It->second.LevelTo, It->second.SourceLoc);

          return nullptr;
        }
        // no user specified level, need to passing -1 to BE.
        Map[-1].Factor = FE;
        Map[-1].LevelFrom = -1;
        Map[-1].LevelTo = -1;
        Map[-1].SourceLoc = SourceLoc;
      }
      if (A.isArgIdent(AI) &&
          A.getArgAsIdent(AI)->Ident->getName() == "private")
        for (AI = AI + 1; AI < A.getNumArgs(); ++AI) {
          if (A.isArgIdent(AI))
            break;
          Privates.push_back(A.getArgAsExpr(AI));
        }
    }
  SmallVector<int , 2> Levels;
  SmallVector<Expr *, 2> Factors;
  for (int I = -1; I <= 8; I++) {
    MapType::iterator It = Map.find(I);
    if (It != Map.end()) {
      Factors.push_back(It->second.Factor);
      Levels.push_back(I);
    }
  }
  const IntelBlockLoopAttr *BL = IntelBlockLoopAttr::CreateImplicit(
      S.Context, Factors.data(), Factors.size(), Levels.data(), Levels.size(),
      Privates.data(), Privates.size(), AA);
  if (!S.CheckIntelBlockLoopAttribute(BL))
    return nullptr;
  return const_cast<IntelBlockLoopAttr *>(BL);
}

static bool CheckBlockLoopScalarExpr(const Expr *E, Sema &S) {
  assert(E && "Invalid expression");
  QualType QT = E->getType();
  if (!QT->isScalarType()) {
    S.Diag(E->getExprLoc(), diag::err_pragma_loop_invalid_argument) << QT;
    return false;
  }
  return true;
}

static bool CheckBlockLoopIntegerExpr(const Expr *E, Sema &S) {
  assert(E && "Invalid expression");
  QualType QT = E->getType();
  if (!QT->isIntegralOrEnumerationType()) {
    S.Diag(E->getExprLoc(), diag::err_pragma_loop_invalid_argument_type) << QT;
    return false;
  }
  return true;
}

// Check expressions and create new attribute for block_loop.
bool Sema::CheckIntelBlockLoopAttribute(const IntelBlockLoopAttr *BL) {

  if (this->CurContext->isDependentContext())
    return true;

  SmallVector<std::pair <const VarDecl *, const Expr *>, 2> PrivateVar;
  for (const auto *P : BL->privates()) {
    if (!CheckBlockLoopScalarExpr(P, *this))
      return false;
    const VarDecl *VD = nullptr;
    if (const DeclRefExpr *DE = dyn_cast_or_null<DeclRefExpr>(P))
      VD = dyn_cast_or_null<VarDecl>(DE->getDecl());
    if (!VD) {
      Diag(P->getExprLoc(),
           diag::err_pragma_block_loop_private_expected_var_arg)
          << VD;
      return false;
    }
    PrivateVar.push_back(std::make_pair(VD, P));
  }
  for (auto *F : BL->factors()) {
    if (F) {
      if (!CheckBlockLoopIntegerExpr(F, *this))
        return false;
      if (!PrivateVar.empty()) {
        llvm::APSInt ValueAPS;
        ExprResult Res = VerifyIntegerConstantExpression(F, &ValueAPS);
        if (Res.isInvalid())
          return false;
      }
    }
  }
  // check duplicate variables are used in private clauses.
  using PrivateVarType = std::pair <const VarDecl *, const Expr *>;
  stable_sort(PrivateVar.begin(), PrivateVar.end(),
              [](const PrivateVarType &LHS, const PrivateVarType &RHS) {
                return LHS.first < RHS.first;
              });
  SmallVector<PrivateVarType, 4>::iterator Found = std::adjacent_find(
      begin(PrivateVar), end(PrivateVar),
      [](const PrivateVarType &LHS, const PrivateVarType &RHS) {
        return LHS.first == RHS.first;
      });
  if (Found != PrivateVar.end()) {
    Diag(Found->second->getExprLoc(),
           diag::err_duplicate_variable_name)
        << Found->first;
    if (Found->second != (Found + 1)->second)
      Diag((Found + 1)->second->getExprLoc(),
             diag::note_omp_referenced)
          << (Found + 1)->first;
    return false;
  }
  return true;
}
#endif // INTEL_CUSTOMIZATION

namespace {
class CallExprFinder : public ConstEvaluatedExprVisitor<CallExprFinder> {
  bool FoundCallExpr = false;

public:
  typedef ConstEvaluatedExprVisitor<CallExprFinder> Inherited;

  CallExprFinder(Sema &S, const Stmt *St) : Inherited(S.Context) { Visit(St); }

  bool foundCallExpr() { return FoundCallExpr; }

  void VisitCallExpr(const CallExpr *E) { FoundCallExpr = true; }

  void Visit(const Stmt *St) {
    if (!St)
      return;
    ConstEvaluatedExprVisitor<CallExprFinder>::Visit(St);
  }
};
} // namespace

static Attr *handleNoMergeAttr(Sema &S, Stmt *St, const ParsedAttr &A,
                               SourceRange Range) {
  NoMergeAttr NMA(S.Context, A);
  if (S.CheckAttrNoArgs(A))
    return nullptr;

  CallExprFinder CEF(S, St);

  if (!CEF.foundCallExpr()) {
    S.Diag(St->getBeginLoc(), diag::warn_nomerge_attribute_ignored_in_stmt)
        << NMA.getSpelling();
    return nullptr;
  }

  return ::new (S.Context) NoMergeAttr(S.Context, A);
}

static void
CheckForIncompatibleAttributes(Sema &S,
                               const SmallVectorImpl<const Attr *> &Attrs) {
  // There are 6 categories of loop hints attributes: vectorize, interleave,
  // unroll, unroll_and_jam, pipeline and distribute. Except for distribute they
  // come in two variants: a state form and a numeric form.  The state form
  // selectively defaults/enables/disables the transformation for the loop
  // (for unroll, default indicates full unrolling rather than enabling the
  // transformation). The numeric form form provides an integer hint (for
  // example, unroll count) to the transformer. The following array accumulates
  // the hints encountered while iterating through the attributes to check for
  // compatibility.
  struct {
    const LoopHintAttr *StateAttr;
    const LoopHintAttr *NumericAttr;
#if INTEL_CUSTOMIZATION
  } HintAttrs[] = {{nullptr, nullptr}, // Vectorize
                   {nullptr, nullptr}, // II
                   {nullptr, nullptr}, // IVDep
                   {nullptr, nullptr}, // IVDepLoop
                   {nullptr, nullptr}, // IVDepBack
                   {nullptr, nullptr}, // ForceHyperopt
                   {nullptr, nullptr}, // Fusion
                   {nullptr, nullptr}, // VectorAlways
                   {nullptr, nullptr}, // LoopCount
                   {nullptr, nullptr}, // LoopCountMin
                   {nullptr, nullptr}, // LoopCountMax
                   {nullptr, nullptr}, // LoopCountAvg
                   {nullptr, nullptr}, // Interleave
                   {nullptr, nullptr}, // Unroll
                   {nullptr, nullptr}, // UnrollAndJam
                   {nullptr, nullptr}, // Pipeline
                   {nullptr, nullptr}, // Distribute
                   {nullptr, nullptr}};// Vectorize Predicate
#endif // INTEL_CUSTOMIZATION

  for (const auto *I : Attrs) {
    const LoopHintAttr *LH = dyn_cast<LoopHintAttr>(I);

    // Skip non loop hint attributes
    if (!LH)
      continue;

    LoopHintAttr::OptionType Option = LH->getOption();
#if INTEL_CUSTOMIZATION
    enum {
      Vectorize,
      II,
      IVDep,
      IVDepLoop,
      IVDepBack,
      ForceHyperopt,
      Fusion,
      VectorAlways,
      LoopCount,
      LoopCountMin,
      LoopCountMax,
      LoopCountAvg,
      Interleave,
      Unroll,
      UnrollAndJam,
      Distribute,
      Pipeline,
      VectorizePredicate
    } Category;
#endif // INTEL_CUSTOMIZATION
    switch (Option) {
#if INTEL_CUSTOMIZATION
    case LoopHintAttr::IIAtMost:
    case LoopHintAttr::IIAtLeast:
    case LoopHintAttr::MinIIAtFmax:
      Category = II;
      break;
    case LoopHintAttr::IVDep:
    case LoopHintAttr::IVDepHLS:
    case LoopHintAttr::IVDepHLSIntel:
      Category = IVDep;
      break;
    case LoopHintAttr::IVDepLoop:
      Category = IVDepLoop;
      break;
    case LoopHintAttr::IVDepBack:
      Category = IVDepBack;
      break;
    case LoopHintAttr::ForceHyperopt:
      Category = ForceHyperopt;
      break;
    case LoopHintAttr::Fusion:
      Category = Fusion;
      break;
    case LoopHintAttr::VectorizeAlways:
      Category = VectorAlways;
      break;
    case LoopHintAttr::LoopCount:
      Category = LoopCount;
      break;
    case LoopHintAttr::LoopCountMax:
      Category = LoopCountMax;
      break;
    case LoopHintAttr::LoopCountMin:
      Category = LoopCountMin;
      break;
    case LoopHintAttr::LoopCountAvg:
      Category = LoopCountAvg;
      break;
#endif // INTEL_CUSTOMIZATION
    case LoopHintAttr::Vectorize:
    case LoopHintAttr::VectorizeWidth:
      Category = Vectorize;
      break;
    case LoopHintAttr::Interleave:
    case LoopHintAttr::InterleaveCount:
      Category = Interleave;
      break;
    case LoopHintAttr::Unroll:
    case LoopHintAttr::UnrollCount:
      Category = Unroll;
      break;
    case LoopHintAttr::UnrollAndJam:
    case LoopHintAttr::UnrollAndJamCount:
      Category = UnrollAndJam;
      break;
    case LoopHintAttr::Distribute:
      // Perform the check for duplicated 'distribute' hints.
      Category = Distribute;
      break;
    case LoopHintAttr::PipelineDisabled:
    case LoopHintAttr::PipelineInitiationInterval:
      Category = Pipeline;
      break;
    case LoopHintAttr::VectorizePredicate:
      Category = VectorizePredicate;
      break;
    };

    assert(Category < sizeof(HintAttrs) / sizeof(HintAttrs[0]));
    auto &CategoryState = HintAttrs[Category];
    const LoopHintAttr *PrevAttr;
#if INTEL_CUSTOMIZATION
    // To make the code more readable and to limit conflicts handle all
    // Intel-added pragmas in this block. For each pragma:
    //  If it contains a numeric value set PrevAttr and
    //    CategoryState.NumericAttr.
    //  If it contains state (enable/disable) set PrevAttr and
    //    CategoryState.StateAttr.
    //  If the attribute cannot conflict set PrevAttr to nullptr.
    //  If you want a diagnostic if both state and numeric are used set
    //    the Category to Unroll and the community code will take care of it.
    if (Option == LoopHintAttr::IVDep || Option == LoopHintAttr::IVDepLoop ||
        Option == LoopHintAttr::IVDepBack ||
        Option == LoopHintAttr::IVDepHLS ||
        Option == LoopHintAttr::IVDepHLSIntel) {
      switch (LH->getState()) {
      case LoopHintAttr::Numeric:
        // safelen only - don't diagnose
        // for duplicate directives on redundant IVDepHLS pragma
        PrevAttr = nullptr;
        break;
      case LoopHintAttr::LoopExpr:
      case LoopHintAttr::Full:
        // array alone or with safelen - multiple ivdeps with array clauses
        // is okay.
        PrevAttr = nullptr;
        break;
      case LoopHintAttr::Enable:
        // Just #pragma ivdep
        PrevAttr = CategoryState.StateAttr;
        CategoryState.StateAttr = LH;
        break;
      case LoopHintAttr::Disable:
      case LoopHintAttr::AssumeSafety:
        llvm_unreachable("unexpected ivdep state");
      }
    } else if (Option == LoopHintAttr::ForceHyperopt) {
      assert(LH->getState() == LoopHintAttr::Enable ||
             LH->getState() == LoopHintAttr::Disable);
      PrevAttr = CategoryState.StateAttr;
      CategoryState.StateAttr = LH;
      if (PrevAttr) {
        // Diagnose this here to get incompatible error instead of
        // duplicate in cases with 'no' variants.
        PrintingPolicy Policy(S.Context.getLangOpts());
        SourceLocation OptionLoc = LH->getRange().getBegin();
        bool Duplicate = PrevAttr->getState() == LH->getState();
        S.Diag(OptionLoc, diag::err_pragma_loop_compatibility)
          << Duplicate << PrevAttr->getDiagnosticName(Policy)
          << LH->getDiagnosticName(Policy);
        PrevAttr = nullptr; // Prevent additional diagnostics.
      }
    } else if (Option == LoopHintAttr::IIAtMost ||
               Option == LoopHintAttr::IIAtLeast ||
               Option == LoopHintAttr::MinIIAtFmax ||
               Option == LoopHintAttr::VectorizeAlways ||
               Option == LoopHintAttr::LoopCount ||
               Option == LoopHintAttr::LoopCountMin ||
               Option == LoopHintAttr::LoopCountMax ||
               Option == LoopHintAttr::LoopCountAvg ||
               Option == LoopHintAttr::Fusion) {
      switch (LH->getState()) {
      case LoopHintAttr::Numeric:
        PrevAttr = nullptr;
        if (Option == LoopHintAttr::LoopCount &&  CategoryState.NumericAttr) {
          SourceLocation OptionLoc = LH->getRange().getBegin();
          SourceLocation PrevOptionLoc =
              CategoryState.NumericAttr->getRange().getBegin();
          // Allow multiple loop counts with same optionLoc only
          if (PrevOptionLoc != OptionLoc)
            PrevAttr = CategoryState.NumericAttr;
        } else
        PrevAttr = CategoryState.NumericAttr;
        CategoryState.NumericAttr = LH;
        break;
      case LoopHintAttr::Enable:
        PrevAttr = CategoryState.StateAttr;
        CategoryState.StateAttr = LH;
        break;
      case LoopHintAttr::Disable:
        PrevAttr = CategoryState.StateAttr;
        CategoryState.StateAttr = LH;
        break;
      case LoopHintAttr::LoopExpr:
      case LoopHintAttr::Full:
      case LoopHintAttr::AssumeSafety:
        llvm_unreachable("unexpected loop pragma state");
      }
      if (Option == LoopHintAttr::MinIIAtFmax &&
          CategoryState.NumericAttr) {
        PrevAttr = CategoryState.NumericAttr;
        CategoryState.NumericAttr = LH;
      }
    } else
#endif // INTEL_CUSTOMIZATION
    if (Option == LoopHintAttr::Vectorize ||
        Option == LoopHintAttr::Interleave || Option == LoopHintAttr::Unroll ||
        Option == LoopHintAttr::UnrollAndJam ||
        Option == LoopHintAttr::VectorizePredicate ||
        Option == LoopHintAttr::PipelineDisabled ||
        Option == LoopHintAttr::Distribute) {
      // Enable|Disable|AssumeSafety hint.  For example, vectorize(enable).
      PrevAttr = CategoryState.StateAttr;
      CategoryState.StateAttr = LH;
    } else {
      // Numeric hint.  For example, vectorize_width(8).
      PrevAttr = CategoryState.NumericAttr;
      CategoryState.NumericAttr = LH;
    }

    PrintingPolicy Policy(S.Context.getLangOpts());
    SourceLocation OptionLoc = LH->getRange().getBegin();
    if (PrevAttr)
      // Cannot specify same type of attribute twice.
      S.Diag(OptionLoc, diag::err_pragma_loop_compatibility)
          << /*Duplicate=*/true << PrevAttr->getDiagnosticName(Policy)
          << LH->getDiagnosticName(Policy);

    if (CategoryState.StateAttr && CategoryState.NumericAttr &&
        (Category == Unroll || Category == UnrollAndJam ||
         CategoryState.StateAttr->getState() == LoopHintAttr::Disable)) {
      // Disable hints are not compatible with numeric hints of the same
      // category.  As a special case, numeric unroll hints are also not
      // compatible with enable or full form of the unroll pragma because these
      // directives indicate full unrolling.
      S.Diag(OptionLoc, diag::err_pragma_loop_compatibility)
          << /*Duplicate=*/false
          << CategoryState.StateAttr->getDiagnosticName(Policy)
          << CategoryState.NumericAttr->getDiagnosticName(Policy);
    }
  }
}

template <typename LoopAttrT>
static void CheckForDuplicationSYCLLoopAttribute(
    Sema &S, const SmallVectorImpl<const Attr *> &Attrs, SourceRange Range,
    bool isIntelFPGAAttr = true) {
  const LoopAttrT *LoopAttr = nullptr;

  for (const auto *I : Attrs) {
    if (LoopAttr) {
      if (isa<LoopAttrT>(I)) {
        SourceLocation Loc = Range.getBegin();
        // Cannot specify same type of attribute twice.
        S.Diag(Loc, diag::err_sycl_loop_attr_duplication)
            << isIntelFPGAAttr << LoopAttr->getName();
      }
    }
    if (isa<LoopAttrT>(I))
      LoopAttr = cast<LoopAttrT>(I);
  }
}

/// Diagnose mutually exclusive attributes when present on a given
/// declaration. Returns true if diagnosed.
template <typename LoopAttrT, typename LoopAttrT2>
static void CheckMutualExclusionSYCLLoopAttribute(
    Sema &S, const SmallVectorImpl<const Attr *> &Attrs, SourceRange Range) {
  const LoopAttrT *LoopAttr = nullptr;
  const LoopAttrT2 *LoopAttr2 = nullptr;

  for (const auto *I : Attrs) {
    if (isa<LoopAttrT>(I))
      LoopAttr = cast<LoopAttrT>(I);
    if (isa<LoopAttrT2>(I))
      LoopAttr2 = cast<LoopAttrT2>(I);
    if (LoopAttr && LoopAttr2) {
      #if INTEL_CUSTOMIZATION
      StringRef LoopAttrName = isa<LoopAttrT>(LoopAttr)
                               ? LoopAttr->getName()
                               : LoopAttr->getSpelling();
      StringRef LoopAttr2Name = isa<LoopAttrT2>(LoopAttr2)
                                ? LoopAttr2->getName()
                                :LoopAttr2->getSpelling();
      S.Diag(Range.getBegin(), diag::err_attributes_are_not_compatible)
          << LoopAttrName << LoopAttr2Name;
      #endif // INTEL_CUSTOMIZATION
    }
  }
}

static void CheckForIncompatibleSYCLLoopAttributes(
    Sema &S, const SmallVectorImpl<const Attr *> &Attrs, SourceRange Range) {
  CheckForDuplicationSYCLLoopAttribute<SYCLIntelFPGAIIAttr>(S, Attrs, Range);
  CheckForDuplicationSYCLLoopAttribute<SYCLIntelFPGAMaxConcurrencyAttr>(
      S, Attrs, Range);
  CheckForDuplicationSYCLLoopAttribute<SYCLIntelFPGALoopCoalesceAttr>(S, Attrs,
                                                                      Range);
  CheckForDuplicationSYCLLoopAttribute<SYCLIntelFPGADisableLoopPipeliningAttr>(
      S, Attrs, Range);
  CheckForDuplicationSYCLLoopAttribute<SYCLIntelFPGAMaxInterleavingAttr>(
      S, Attrs, Range);
  CheckForDuplicationSYCLLoopAttribute<SYCLIntelFPGASpeculatedIterationsAttr>(
      S, Attrs, Range);
  CheckForDuplicationSYCLLoopAttribute<LoopUnrollHintAttr>(S, Attrs, Range,
                                                           false);
  CheckMutualExclusionSYCLLoopAttribute<SYCLIntelFPGADisableLoopPipeliningAttr,
                                        SYCLIntelFPGAMaxInterleavingAttr>(
      S, Attrs, Range);
  CheckMutualExclusionSYCLLoopAttribute<SYCLIntelFPGADisableLoopPipeliningAttr,
                                        SYCLIntelFPGASpeculatedIterationsAttr>(
      S, Attrs, Range);
  CheckMutualExclusionSYCLLoopAttribute<SYCLIntelFPGADisableLoopPipeliningAttr,
                                        SYCLIntelFPGAIIAttr>(S, Attrs, Range);
  CheckMutualExclusionSYCLLoopAttribute<SYCLIntelFPGADisableLoopPipeliningAttr,
                                        SYCLIntelFPGAIVDepAttr>(S, Attrs,
                                                                Range);
  CheckMutualExclusionSYCLLoopAttribute<SYCLIntelFPGADisableLoopPipeliningAttr,
                                        SYCLIntelFPGAMaxConcurrencyAttr>(
      S, Attrs, Range);

  CheckRedundantSYCLIntelFPGAIVDepAttrs(S, Attrs);
}

void CheckForIncompatibleUnrollHintAttributes(
    Sema &S, const SmallVectorImpl<const Attr *> &Attrs, SourceRange Range) {

  // This check is entered after it was analyzed that there are no duplicating
  // pragmas and loop attributes. So, let's perform check that there are no
  // conflicting pragma unroll and unroll attribute for the loop.
  const LoopUnrollHintAttr *AttrUnroll = nullptr;
  const LoopHintAttr *PragmaUnroll = nullptr;
  for (const auto *I : Attrs) {
    if (auto *LH = dyn_cast<LoopUnrollHintAttr>(I))
      AttrUnroll = LH;
    if (auto *LH = dyn_cast<LoopHintAttr>(I)) {
      LoopHintAttr::OptionType Opt = LH->getOption();
      if (Opt == LoopHintAttr::Unroll || Opt == LoopHintAttr::UnrollCount)
        PragmaUnroll = LH;
    }
  }

  if (AttrUnroll && PragmaUnroll) {
    PrintingPolicy Policy(S.Context.getLangOpts());
    SourceLocation Loc = Range.getBegin();
    S.Diag(Loc, diag::err_loop_unroll_compatibility)
        << PragmaUnroll->getDiagnosticName(Policy)
        << AttrUnroll->getDiagnosticName(Policy);
  }
}

#if INTEL_CUSTOMIZATION
// Emit incompatible error for #pragma ii_at_most, #pragma ii_at_least,
// #pragma min_ii_at_target_fmax, and #pragma force_hyperopt with
// #pragma diasable_loop_pipelining.
void CheckForIncompatibleHLSAttributes(
    Sema &S, const SmallVectorImpl<const Attr *> &Attrs, SourceRange Range) {
  const SYCLIntelFPGADisableLoopPipeliningAttr *PragmaDisable = nullptr;
  const LoopHintAttr *PragmaLoopHint = nullptr;

  for (const auto *I : Attrs) {
    if (auto *LH = dyn_cast<const SYCLIntelFPGADisableLoopPipeliningAttr>(I))
      PragmaDisable = LH;
    if (auto *LH = dyn_cast<LoopHintAttr>(I)) {
      LoopHintAttr::OptionType Opt = LH->getOption();
      if ((Opt == LoopHintAttr::IIAtMost) ||
          (Opt == LoopHintAttr::IIAtLeast) ||
          (Opt == LoopHintAttr::MinIIAtFmax) ||
          (Opt == LoopHintAttr::ForceHyperopt))
        PragmaLoopHint = LH;
    }
    if (PragmaLoopHint && PragmaDisable) {
      PrintingPolicy Policy(S.Context.getLangOpts());
      SourceLocation OptionLoc = I->getRange().getBegin();
      S.Diag(OptionLoc, diag::err_pragma_loop_compatibility)
          << /*Duplicate=*/false << PragmaDisable->getDiagnosticName(Policy)
          << PragmaLoopHint->getDiagnosticName(Policy);
    }
  }
}

// Emit duplicate error for #pragma ii_at_most, #pragma ii_at_least,
// and #pragma min_ii_at_target_fmax with #pragma ii.
void CheckForDuplicateHLSAttributes(
    Sema &S, const SmallVectorImpl<const Attr *> &Attrs, SourceRange Range) {
  const SYCLIntelFPGAIIAttr *PragmaII = nullptr;
  const LoopHintAttr *PragmaLoopHint = nullptr;

  for (const auto *I : Attrs) {
    if (auto *LH = dyn_cast<const SYCLIntelFPGAIIAttr>(I))
      PragmaII = LH;
    if (auto *LH = dyn_cast<LoopHintAttr>(I)) {
      LoopHintAttr::OptionType Opt = LH->getOption();
      if ((Opt == LoopHintAttr::IIAtMost) ||
          (Opt == LoopHintAttr::IIAtLeast) ||
          (Opt == LoopHintAttr::MinIIAtFmax))
        PragmaLoopHint = LH;
    }
    if (PragmaLoopHint && PragmaII) {
      PrintingPolicy Policy(S.Context.getLangOpts());
      SourceLocation OptionLoc = I->getRange().getBegin();
      S.Diag(OptionLoc, diag::err_pragma_loop_compatibility)
          << /*Duplicate=*/true << PragmaII->getDiagnosticName(Policy)
          << PragmaLoopHint->getDiagnosticName(Policy);
    }
  }
}
#endif // INTEL_CUSTOMIZATION

static bool CheckLoopUnrollAttrExpr(Sema &S, Expr *E,
                                    const AttributeCommonInfo &A,
                                    unsigned *UnrollFactor = nullptr) {
  if (E && !E->isInstantiationDependent()) {
    llvm::APSInt ArgVal(32);

    if (!E->isIntegerConstantExpr(ArgVal, S.Context))
      return S.Diag(E->getExprLoc(), diag::err_attribute_argument_type)
             << A.getAttrName() << AANT_ArgumentIntegerConstant
             << E->getSourceRange();

    if (ArgVal.isNonPositive())
      return S.Diag(E->getExprLoc(),
                    diag::err_attribute_requires_positive_integer)
             << A.getAttrName() << /* positive */ 0;

    if (UnrollFactor)
      *UnrollFactor = ArgVal.getZExtValue();
  }
  return false;
}

LoopUnrollHintAttr *Sema::BuildLoopUnrollHintAttr(const AttributeCommonInfo &A,
                                                  Expr *E) {
  return !CheckLoopUnrollAttrExpr(*this, E, A)
             ? new (Context) LoopUnrollHintAttr(Context, A, E)
             : nullptr;
}

OpenCLUnrollHintAttr *
Sema::BuildOpenCLLoopUnrollHintAttr(const AttributeCommonInfo &A, Expr *E) {
  unsigned UnrollFactor = 0;
  return !CheckLoopUnrollAttrExpr(*this, E, A, &UnrollFactor)
             ? new (Context) OpenCLUnrollHintAttr(Context, A, UnrollFactor)
             : nullptr;
}

static Attr *handleLoopUnrollHint(Sema &S, Stmt *St, const ParsedAttr &A,
                                  SourceRange Range) {
  // Although the feature was introduced only in OpenCL C v2.0 s6.11.5, it's
  // useful for OpenCL 1.x too and doesn't require HW support.
  // opencl_unroll_hint or clang::unroll can have 0 arguments (compiler
  // determines unrolling factor) or 1 argument (the unroll factor provided
  // by the user).

  unsigned NumArgs = A.getNumArgs();

  if (NumArgs > 1) {
    S.Diag(A.getLoc(), diag::err_attribute_too_many_arguments) << A << 1;
    return nullptr;
  }

  Expr *E = NumArgs ? A.getArgAsExpr(0) : nullptr;
  if (A.getParsedKind() == ParsedAttr::AT_OpenCLUnrollHint)
    return S.BuildOpenCLLoopUnrollHintAttr(A, E);
  else if (A.getParsedKind() == ParsedAttr::AT_LoopUnrollHint)
    return S.BuildLoopUnrollHintAttr(A, E);

  return nullptr;
}

static Attr *ProcessStmtAttribute(Sema &S, Stmt *St, const ParsedAttr &A,
#if INTEL_CUSTOMIZATION
                                  const ParsedAttributesView &AL,
#endif // INTEL_CUSTOMIZATION
                                  SourceRange Range) {
  switch (A.getKind()) {
  case ParsedAttr::UnknownAttribute:
    S.Diag(A.getLoc(), A.isDeclspecAttribute()
                           ? (unsigned)diag::warn_unhandled_ms_attribute_ignored
                           : (unsigned)diag::warn_unknown_attribute_ignored)
        << A;
    return nullptr;
#if INTEL_CUSTOMIZATION
  case ParsedAttr::AT_IntelInline:
    return handleIntelInlineAttr(S, St, A, Range);
  case ParsedAttr::AT_IntelBlockLoop:
    return handleIntelBlockLoopAttr(S, St, A, AL, Range);
  case ParsedAttr::AT_LoopFuse:
    return handleLoopFuseAttr(S, St, A, Range);
#endif // INTEL_CUSTOMIZATION
  case ParsedAttr::AT_FallThrough:
    return handleFallThroughAttr(S, St, A, Range);
  case ParsedAttr::AT_LoopHint:
    return handleLoopHintAttr(S, St, A, Range);
  case ParsedAttr::AT_SYCLIntelFPGAIVDep:
    return handleIntelFPGAIVDepAttr(S, A);
  case ParsedAttr::AT_SYCLIntelFPGAII:
    return handleIntelFPGALoopAttr<SYCLIntelFPGAIIAttr>(S, A);
  case ParsedAttr::AT_SYCLIntelFPGAMaxConcurrency:
    return handleIntelFPGALoopAttr<SYCLIntelFPGAMaxConcurrencyAttr>(S, A);
  case ParsedAttr::AT_SYCLIntelFPGALoopCoalesce:
    return handleIntelFPGALoopAttr<SYCLIntelFPGALoopCoalesceAttr>(S, A);
  case ParsedAttr::AT_SYCLIntelFPGADisableLoopPipelining:
    return handleIntelFPGALoopAttr<SYCLIntelFPGADisableLoopPipeliningAttr>(S,
                                                                           A);
  case ParsedAttr::AT_SYCLIntelFPGAMaxInterleaving:
    return handleIntelFPGALoopAttr<SYCLIntelFPGAMaxInterleavingAttr>(S, A);
  case ParsedAttr::AT_SYCLIntelFPGASpeculatedIterations:
    return handleIntelFPGALoopAttr<SYCLIntelFPGASpeculatedIterationsAttr>(S, A);
  case ParsedAttr::AT_OpenCLUnrollHint:
  case ParsedAttr::AT_LoopUnrollHint:
    return handleLoopUnrollHint(S, St, A, Range);
  case ParsedAttr::AT_Suppress:
    return handleSuppressAttr(S, St, A, Range);
  case ParsedAttr::AT_NoMerge:
    return handleNoMergeAttr(S, St, A, Range);
  default:
    // if we're here, then we parsed a known attribute, but didn't recognize
    // it as a statement attribute => it is declaration attribute
    S.Diag(A.getRange().getBegin(), diag::err_decl_attribute_invalid_on_stmt)
        << A << St->getBeginLoc();
    return nullptr;
  }
}

StmtResult Sema::ProcessStmtAttributes(Stmt *S,
                                       const ParsedAttributesView &AttrList,
                                       SourceRange Range) {
  SmallVector<const Attr*, 8> Attrs;
  for (const ParsedAttr &AL : AttrList) {
#if INTEL_CUSTOMIZATION
    if (Attr *a = ProcessStmtAttribute(*this, S, AL, AttrList,  Range))
#endif // INTEL_CUSTOMIZATION
      Attrs.push_back(a);
  }

  CheckForIncompatibleAttributes(*this, Attrs);
  CheckForIncompatibleSYCLLoopAttributes(*this, Attrs, Range);
  CheckForIncompatibleUnrollHintAttributes(*this, Attrs, Range);
#if INTEL_CUSTOMIZATION
  CheckForIncompatibleHLSAttributes(*this, Attrs, Range);
  CheckForDuplicateHLSAttributes(*this, Attrs, Range);
#endif // INTEL_CUSTOMIZATION

  if (Attrs.empty())
    return S;

  return ActOnAttributedStmt(Range.getBegin(), Attrs, S);
}
bool Sema::CheckRebuiltAttributedStmtAttributes(ArrayRef<const Attr *> Attrs) {
  CheckRedundantSYCLIntelFPGAIVDepAttrs(*this, Attrs);
  return false;
}
