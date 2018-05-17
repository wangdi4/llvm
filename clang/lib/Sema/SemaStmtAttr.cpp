//===--- SemaStmtAttr.cpp - Statement Attribute Handling ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements stmt-related attribute processing.
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/SemaInternal.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Sema/DelayedDiagnostic.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/LoopHint.h"
#include "clang/Sema/ScopeInfo.h"
#include "llvm/ADT/StringExtras.h"

using namespace clang;
using namespace sema;

static Attr *handleFallThroughAttr(Sema &S, Stmt *St, const AttributeList &A,
                                   SourceRange Range) {
  FallThroughAttr Attr(A.getRange(), S.Context,
                       A.getAttributeSpellingListIndex());
  if (!isa<NullStmt>(St)) {
    S.Diag(A.getRange().getBegin(), diag::err_fallthrough_attr_wrong_target)
        << Attr.getSpelling() << St->getLocStart();
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
    S.Diag(A.getLoc(), diag::ext_cxx17_attr) << A.getName();

  FnScope->setHasFallthroughStmt();
  return ::new (S.Context) auto(Attr);
}

static Attr *handleSuppressAttr(Sema &S, Stmt *St, const AttributeList &A,
                                SourceRange Range) {
  if (A.getNumArgs() < 1) {
    S.Diag(A.getLoc(), diag::err_attribute_too_few_arguments)
        << A.getName() << 1;
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
      A.getRange(), S.Context, DiagnosticIdentifiers.data(),
      DiagnosticIdentifiers.size(), A.getAttributeSpellingListIndex());
}

static Attr *handleLoopHintAttr(Sema &S, Stmt *St, const AttributeList &A,
                                SourceRange) {
  IdentifierLoc *PragmaNameLoc = A.getArgAsIdent(0);
  IdentifierLoc *OptionLoc = A.getArgAsIdent(1);
  IdentifierLoc *StateLoc = A.getArgAsIdent(2);
  Expr *ValueExpr = A.getArgAsExpr(3);
#if INTEL_CUSTOMIZATION
  Expr *ArrayExpr = A.getArgAsExpr(4);

  bool PragmaLoopCoalesce = PragmaNameLoc->Ident->getName() == "loop_coalesce";
  bool PragmaII = PragmaNameLoc->Ident->getName() == "ii";
  bool PragmaMaxConcurrency =
      PragmaNameLoc->Ident->getName() == "max_concurrency";
  bool PragmaIVDep = PragmaNameLoc->Ident->getName() == "ivdep";
  bool PragmaDistributePoint =
      PragmaNameLoc->Ident->getName() == "distribute_point";
  bool PragmaNoFusion = PragmaNameLoc->Ident->getName() == "nofusion";
  bool PragmaNoVector = PragmaNameLoc->Ident->getName() == "novector";
  bool NonLoopPragmaDistributePoint =
      PragmaDistributePoint && St->getStmtClass() != Stmt::DoStmtClass &&
      St->getStmtClass() != Stmt::ForStmtClass &&
      St->getStmtClass() != Stmt::CXXForRangeStmtClass &&
      St->getStmtClass() != Stmt::WhileStmtClass;
#endif // INTEL_CUSTOMIZATION
  bool PragmaUnroll = PragmaNameLoc->Ident->getName() == "unroll";
  bool PragmaNoUnroll = PragmaNameLoc->Ident->getName() == "nounroll";
#ifdef INTEL_CUSTOMIZATION
  if (NonLoopPragmaDistributePoint) {
    bool withinLoop = false;
    for (Scope *CS = S.getCurScope(); CS; CS = CS->getParent())
      if (CS->getFlags() & Scope::ContinueScope) {
        withinLoop = true;
        break;
      }
    if (!withinLoop) {
      S.Diag(St->getLocStart(), diag::err_pragma_distpt_on_nonloop_stmt)
          << "#pragma distribute_point";
      return nullptr;
    }
  } else
#endif // INTEL_CUSTOMIZATION
  if (St->getStmtClass() != Stmt::DoStmtClass &&
      St->getStmtClass() != Stmt::ForStmtClass &&
      St->getStmtClass() != Stmt::CXXForRangeStmtClass &&
      St->getStmtClass() != Stmt::WhileStmtClass) {
    const char *Pragma =
        llvm::StringSwitch<const char *>(PragmaNameLoc->Ident->getName())
            .Case("unroll", "#pragma unroll")
            .Case("nounroll", "#pragma nounroll")
#if INTEL_CUSTOMIZATION
            .Case("loop_coalesce", "#pragma loop_coalesce")
            .Case("ii", "#pragma ii")
            .Case("max_concurrency", "#pragma max_concurrency")
            .Case("ivdep", "#pragma ivdep")
            .Case("nofusion", "#pragma nofusion")
            .Case("novector", "#pragma novector")
#endif // INTEL_CUSTOMIZATION
            .Default("#pragma clang loop");
    S.Diag(St->getLocStart(), diag::err_pragma_loop_precedes_nonloop) << Pragma;
    return nullptr;
  }

  LoopHintAttr::Spelling Spelling =
      LoopHintAttr::Spelling(A.getAttributeSpellingListIndex());
  LoopHintAttr::OptionType Option;
  LoopHintAttr::LoopHintState State;
  if (PragmaNoUnroll) {
    // #pragma nounroll
    Option = LoopHintAttr::Unroll;
    State = LoopHintAttr::Disable;
  } else if (PragmaUnroll) {
    if (ValueExpr) {
      // #pragma unroll N
      Option = LoopHintAttr::UnrollCount;
      State = LoopHintAttr::Numeric;
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
          State = Val.getBoolValue() ? LoopHintAttr::Enable : LoopHintAttr::Disable;
          Option = LoopHintAttr::Unroll;
          ValueExpr = nullptr;
        }
      }
#endif // INTEL_CUSTOMIZATION
    } else {
      // #pragma unroll
      Option = LoopHintAttr::Unroll;
      State = LoopHintAttr::Enable;
    }
#if INTEL_CUSTOMIZATION
  } else if (PragmaLoopCoalesce) {
    Option = LoopHintAttr::LoopCoalesce;
    if (ValueExpr != nullptr)
      State = LoopHintAttr::Numeric;
    else
      State = LoopHintAttr::Enable;
  } else if (PragmaII) {
    Option = LoopHintAttr::II;
    State = LoopHintAttr::Numeric;
  } else if (PragmaMaxConcurrency) {
    Option = LoopHintAttr::MaxConcurrency;
    State = LoopHintAttr::Numeric;
  } else if (PragmaIVDep) {
    Option = LoopHintAttr::IVDep;
    if (ValueExpr && ArrayExpr)
      State = LoopHintAttr::Full;
    else if (ValueExpr)
      State = LoopHintAttr::Numeric;
    else if (ArrayExpr)
      State = LoopHintAttr::LoopExpr;
    else
      State = LoopHintAttr::Enable;
  } else if (PragmaDistributePoint) {
    Option = LoopHintAttr::Distribute;
    State = LoopHintAttr::Enable;
  } else if (PragmaNoFusion) {
    Option = LoopHintAttr::NoFusion;
    State = LoopHintAttr::Enable;
  } else if (PragmaNoVector) {
    Option = LoopHintAttr::Vectorize;
    State = LoopHintAttr::Disable;
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
                 .Case("interleave_count", LoopHintAttr::InterleaveCount)
                 .Case("unroll", LoopHintAttr::Unroll)
                 .Case("unroll_count", LoopHintAttr::UnrollCount)
                 .Case("distribute", LoopHintAttr::Distribute)
                 .Default(LoopHintAttr::Vectorize);
    if (Option == LoopHintAttr::VectorizeWidth ||
        Option == LoopHintAttr::InterleaveCount ||
        Option == LoopHintAttr::UnrollCount) {
      assert(ValueExpr && "Attribute must have a valid value expression.");
      if (S.CheckLoopHintExpr(ValueExpr, St->getLocStart()))
        return nullptr;
      State = LoopHintAttr::Numeric;
    } else if (Option == LoopHintAttr::Vectorize ||
               Option == LoopHintAttr::Interleave ||
               Option == LoopHintAttr::Unroll ||
               Option == LoopHintAttr::Distribute) {
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

  return LoopHintAttr::CreateImplicit(S.Context, Spelling, Option, State,
                                      ValueExpr, ArrayExpr, A.getRange());
}

#if INTEL_CUSTOMIZATION
static Attr *handleIntelInlineAttr(Sema &S, Stmt *St, const AttributeList &A,
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
  return IntelInlineAttr::CreateImplicit(
      S.Context,
      static_cast<IntelInlineAttr::Spelling>(A.getAttributeSpellingListIndex()),
      Option, A.getRange());
}
#endif // INTEL_CUSTOMIZATION

static void
CheckForIncompatibleAttributes(Sema &S,
                               const SmallVectorImpl<const Attr *> &Attrs) {
  // There are 4 categories of loop hints attributes: vectorize, interleave,
  // unroll and distribute. Except for distribute they come in two variants: a
  // state form and a numeric form.  The state form selectively
  // defaults/enables/disables the transformation for the loop (for unroll,
  // default indicates full unrolling rather than enabling the transformation).
  // The numeric form form provides an integer hint (for example, unroll count)
  // to the transformer. The following array accumulates the hints encountered
  // while iterating through the attributes to check for compatibility.
  struct {
    const LoopHintAttr *StateAttr;
    const LoopHintAttr *NumericAttr;
  } HintAttrs[] = {{nullptr, nullptr},
#if INTEL_CUSTOMIZATION
                   {nullptr, nullptr},
                   {nullptr, nullptr},
                   {nullptr, nullptr},
                   {nullptr, nullptr},
                   {nullptr, nullptr},
                   {nullptr, nullptr},
#endif // INTEL_CUSTOMIZATION
                   {nullptr, nullptr},
                   {nullptr, nullptr},
                   {nullptr, nullptr}};

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
      LoopCoalesce,
      MaxConcurrency,
      Interleave,
      Unroll,
      Distribute,
      NoFusion,
      NoVector
    } Category;
#endif // INTEL_CUSTOMIZATION
    switch (Option) {
#if INTEL_CUSTOMIZATION
    case LoopHintAttr::II:
      Category = II;
      break;
    case LoopHintAttr::IVDep:
      Category = IVDep;
      break;
    case LoopHintAttr::LoopCoalesce:
      Category = LoopCoalesce;
      break;
    case LoopHintAttr::MaxConcurrency:
      Category = MaxConcurrency;
      break;
    case LoopHintAttr::NoFusion:
      Category = NoFusion;
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
    case LoopHintAttr::Distribute:
      // Perform the check for duplicated 'distribute' hints.
      Category = Distribute;
      break;
    };

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
    if (Option == LoopHintAttr::II || Option == LoopHintAttr::LoopCoalesce ||
        Option == LoopHintAttr::MaxConcurrency ||
        Option == LoopHintAttr::IVDep || Option == LoopHintAttr::NoFusion) {
      switch (LH->getState()) {
      case LoopHintAttr::Numeric:
      case LoopHintAttr::Full:
        // Numeric and Full both contain numeric values.
        PrevAttr = CategoryState.NumericAttr;
        CategoryState.NumericAttr = LH;
        break;
      case LoopHintAttr::Enable:
        PrevAttr = CategoryState.StateAttr;
        CategoryState.StateAttr = LH;
        break;
      case LoopHintAttr::LoopExpr:
        // Multiple ivdeps with array clauses is okay.
        PrevAttr = nullptr;
        break;
      case LoopHintAttr::Disable:
      case LoopHintAttr::AssumeSafety:
        llvm_unreachable("unexpected ivdep state");
      }
      if (Option == LoopHintAttr::LoopCoalesce || Option == LoopHintAttr::IVDep)
        Category = Unroll;
    } else
#endif // INTEL_CUSTOMIZATION
    if (Option == LoopHintAttr::Vectorize ||
        Option == LoopHintAttr::Interleave || Option == LoopHintAttr::Unroll ||
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
        (Category == Unroll ||
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

static Attr *handleOpenCLUnrollHint(Sema &S, Stmt *St, const AttributeList &A,
                                    SourceRange Range) {
  // Although the feature was introduced only in OpenCL C v2.0 s6.11.5, it's
  // useful for OpenCL 1.x too and doesn't require HW support.
  // opencl_unroll_hint can have 0 arguments (compiler
  // determines unrolling factor) or 1 argument (the unroll factor provided
  // by the user).

  unsigned NumArgs = A.getNumArgs();

  if (NumArgs > 1) {
    S.Diag(A.getLoc(), diag::err_attribute_too_many_arguments) << A.getName()
                                                               << 1;
    return nullptr;
  }

  unsigned UnrollFactor = 0;

  if (NumArgs == 1) {
    Expr *E = A.getArgAsExpr(0);
    llvm::APSInt ArgVal(32);

    if (!E->isIntegerConstantExpr(ArgVal, S.Context)) {
      S.Diag(A.getLoc(), diag::err_attribute_argument_type)
          << A.getName() << AANT_ArgumentIntegerConstant << E->getSourceRange();
      return nullptr;
    }

    int Val = ArgVal.getSExtValue();

    if (Val <= 0) {
      S.Diag(A.getRange().getBegin(),
             diag::err_attribute_requires_positive_integer)
          << A.getName();
      return nullptr;
    }
    UnrollFactor = Val;
  }

  return OpenCLUnrollHintAttr::CreateImplicit(S.Context, UnrollFactor);
}

static Attr *ProcessStmtAttribute(Sema &S, Stmt *St, const AttributeList &A,
                                  SourceRange Range) {
  switch (A.getKind()) {
  case AttributeList::UnknownAttribute:
    S.Diag(A.getLoc(), A.isDeclspecAttribute() ?
           diag::warn_unhandled_ms_attribute_ignored :
           diag::warn_unknown_attribute_ignored) << A.getName();
    return nullptr;
#if INTEL_CUSTOMIZATION
  case AttributeList::AT_IntelInline:
    return handleIntelInlineAttr(S, St, A, Range);
#endif // INTEL_CUSTOMIZATION
  case AttributeList::AT_FallThrough:
    return handleFallThroughAttr(S, St, A, Range);
  case AttributeList::AT_LoopHint:
    return handleLoopHintAttr(S, St, A, Range);
  case AttributeList::AT_OpenCLUnrollHint:
    return handleOpenCLUnrollHint(S, St, A, Range);
  case AttributeList::AT_Suppress:
    return handleSuppressAttr(S, St, A, Range);
  default:
    // if we're here, then we parsed a known attribute, but didn't recognize
    // it as a statement attribute => it is declaration attribute
#if INTEL_CUSTOMIZATION
    // CQ#370092 - emit a warning, not error in IntelCompat mode
    if (S.getLangOpts().IntelCompat)
      S.Diag(A.getRange().getBegin(), diag::warn_decl_attribute_invalid_on_stmt)
        << A.getName() << St->getLocStart();
    else
#endif // INTEL_CUSTOMIZATION
    S.Diag(A.getRange().getBegin(), diag::err_decl_attribute_invalid_on_stmt)
        << A.getName() << St->getLocStart();
    return nullptr;
  }
}

StmtResult Sema::ProcessStmtAttributes(Stmt *S, AttributeList *AttrList,
                                       SourceRange Range) {
  SmallVector<const Attr*, 8> Attrs;
  for (const AttributeList* l = AttrList; l; l = l->getNext()) {
    if (Attr *a = ProcessStmtAttribute(*this, S, *l, Range))
      Attrs.push_back(a);
  }

  CheckForIncompatibleAttributes(*this, Attrs);

  if (Attrs.empty())
    return S;

  return ActOnAttributedStmt(Range.getBegin(), Attrs, S);
}
