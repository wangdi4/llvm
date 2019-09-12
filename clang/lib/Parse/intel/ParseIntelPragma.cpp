//===--- ParseIntelPragma.cpp - Intel pragma parsing ------------*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
#include "clang/Parse/RAIIObjectsForParser.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseDiagnostic.h"
#include "clang/Parse/Parser.h"
#include "clang/Sema/Scope.h"
#include "llvm/ADT/StringSwitch.h"

using namespace clang;

namespace {
#include "intel/ParsePragma.h"
}

template<unsigned N>
static void EnterTokenS(Preprocessor &PP, SmallVector<Token, N> Tokens,
                 bool  DisableMacroExpansion){
  auto Size = Tokens.size();
  auto Toks = std::unique_ptr<Token[]>(new Token[Size]);
  for (unsigned i=0; i < Size; i++)
    Toks[i] = Tokens [i];
  PP.EnterTokenStream(std::move(Toks), Size, DisableMacroExpansion, /*IsReinject*/ false);
}

// #pragma inline [recursive]
// #pragma forceinline [recursive]
// #pragma noinline

namespace {
struct PragmaInlineInfo {
  Token PragmaName;
  Token Option;
};
} // end anonymous namespace

bool Parser::HandlePragmaIntelInline(SourceRange &Range,
                                     IdentifierLoc* &KindLoc,
                                     IdentifierLoc* &OptionsLoc) {
  assert(Tok.is(tok::annot_pragma_inline));
  PragmaInlineInfo *Info =
      static_cast<PragmaInlineInfo *>(Tok.getAnnotationValue());

  IdentifierInfo *PragmaNameInfo = Info->PragmaName.getIdentifierInfo();
  KindLoc = IdentifierLoc::create(Actions.Context,
      Info->PragmaName.getLocation(), PragmaNameInfo);

  if (Info->Option.isNot(tok::eod)) {
    IdentifierInfo *OptionInfo = Info->Option.getIdentifierInfo();
    OptionsLoc = IdentifierLoc::create(Actions.Context,
        Info->Option.getLocation(), OptionInfo);
    Range = SourceRange(Info->PragmaName.getLocation(),
                        Info->Option.getLocation());
  } else {
    OptionsLoc = nullptr;
    Range = SourceRange(Info->PragmaName.getLocation());
  }

  return true;
}

StmtResult Parser::ParsePragmaInline(StmtVector &Stmts,
                                     ParsedStmtContext StmtCtx,
                                     SourceLocation *TrailingElseLoc,
                                     ParsedAttributesWithRange &Attrs) {
  // Create temporary attribute list.
  ParsedAttributesWithRange TempAttrs(AttrFactory);

  // Get #pragma inline info and consume annotated token.
  while (Tok.is(tok::annot_pragma_inline)) {
    SourceRange Range;
    IdentifierLoc *KindLoc;
    IdentifierLoc *OptionsLoc;

    HandlePragmaIntelInline(Range, KindLoc, OptionsLoc);

    ArgsUnion Args[] = {KindLoc, OptionsLoc};
    TempAttrs.addNew(KindLoc->Ident, Range, nullptr,
                     KindLoc->Loc, Args, 2,
                     ParsedAttr::AS_Pragma);

    assert(Tok.is(tok::annot_pragma_inline));
    ConsumeAnnotationToken();  // annot_pragma_inline
    while (Tok.isNot(tok::annot_pragma_end))
      ConsumeToken();
    assert(Tok.is(tok::annot_pragma_end));
    ConsumeAnnotationToken(); // annot_pragma_end
  }

  // Get the next statement.
  MaybeParseCXX11Attributes(Attrs);

  StmtResult S = ParseStatementOrDeclarationAfterAttributes(
      Stmts, StmtCtx, TrailingElseLoc, Attrs);

  Attrs.takeAllFrom(TempAttrs);
  return S;
}

void PragmaInlineHandler::HandlePragma(Preprocessor &PP,
                                       PragmaIntroducer Introducer,
                                       Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation InlineLoc = FirstTok.getLocation();
  PragmaInlineInfo *Info =
    new (PP.getPreprocessorAllocator()) PragmaInlineInfo;

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_inline);
  Tok.setLocation(InlineLoc);
  Tok.setAnnotationValue(static_cast<void*>(Info));
  Tokens.push_back(Tok);

  Info->PragmaName = FirstTok;
  PP.Lex(Tok);
  Info->Option = Tok;
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  InlineLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(InlineLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

namespace {
struct PragmaBlockLoopInfo {
  Token PragmaName;
  Token Factor;
  Token Level;
  Token Private;
  ArrayRef<Token> LastToks;
  SmallVector<ArrayRef<Token>, 2> Levels;
  ArrayRef<Token> Factors;
  SmallVector<ArrayRef<Token>, 2> Privates;
};
} // end anonymous namespace

bool Parser::HandlePragmaBlockLoop(ArgsVector *ArgExprs) {
  assert(Tok.is(tok::annot_pragma_blockloop));
  PragmaBlockLoopInfo *Info =
      static_cast<PragmaBlockLoopInfo *>(Tok.getAnnotationValue());
  if (!Info->Factors.empty()) {
    ArgExprs->push_back(IdentifierLoc::create(Actions.Context,
                                             Info->Factor.getLocation(),
                                             Info->Factor.getIdentifierInfo()));
    PP.EnterTokenStream(Info->Factors, /*DisableMacroExpansion=*/false, /*IsReinject*/ false);
    ConsumeAnnotationToken();
    ExprResult VarExpr =
        Actions.CorrectDelayedTyposInExpr(ParseExpression());
    if (VarExpr.isUsable())
      ArgExprs->push_back(VarExpr.get());
  }
  if (!Info->Levels.empty()) {
    ArgExprs->push_back(IdentifierLoc::create(Actions.Context,
                                              Info->Level.getLocation(),
                                              Info->Level.getIdentifierInfo()));
    for (auto &L : Info->Levels) {
      PP.EnterTokenStream(L, /*DisableMacroExpansion=*/false, /*IsReinject*/ false);
      if (Tok.is(tok::annot_pragma_blockloop))
        ConsumeAnnotationToken();
      else
        ConsumeToken();  // The terminator eof.
      ExprResult R = ParseConstantExpression();
      llvm::APSInt Val;
      if (!R.isInvalid())
        R = Actions.VerifyIntegerConstantExpression(R.get(), &Val);
      if (R.isInvalid()) {
        ConsumeToken();  // The terminator eof.
        return false;
      }
      ArgExprs->push_back(R.get());
    }
  }
  if (!Info->Privates.empty()) {
    ArgExprs->push_back(
        IdentifierLoc::create(Actions.Context, Info->Private.getLocation(),
                              Info->Private.getIdentifierInfo()));
    for (auto &P : Info->Privates) {
      PP.EnterTokenStream(P, /*DisableMacroExpansion=*/false, /*IsReinject*/false);
      if (Tok.is(tok::annot_pragma_blockloop))
        ConsumeAnnotationToken();
      else
        ConsumeToken();  // The terminator eof.
      ExprResult VarExpr =
          Actions.CorrectDelayedTyposInExpr(ParseAssignmentExpression());
      if (VarExpr.isUsable())
        ArgExprs->push_back(VarExpr.get());
    }
  }
  if (Info->LastToks.empty()) {
    ConsumeAnnotationToken();
  } else {
    ConsumeToken();  // The terminator eof.
  }
  return true;
}

StmtResult Parser::ParsePragmaBlockLoop(StmtVector &Stmts,
                                        ParsedStmtContext StmtCtx,
                                        SourceLocation *TrailingElseLoc,
                                        ParsedAttributesWithRange &Attrs) {
  // Create temporary attribute list.
  ParsedAttributesWithRange TempAttrs(AttrFactory);

  // Get blockloop arguments and consume annotated token.
  assert(Tok.is(tok::annot_pragma_blockloop));

  while (Tok.is(tok::annot_pragma_blockloop)) {
    PragmaBlockLoopInfo *Info =
        static_cast<PragmaBlockLoopInfo *>(Tok.getAnnotationValue());
    IdentifierInfo *PragmaNameInfo = Info->PragmaName.getIdentifierInfo();
    IdentifierLoc *PragmaNameLoc = IdentifierLoc::create(
        Actions.Context, Info->PragmaName.getLocation(), PragmaNameInfo);
    ArgsVector ArgExprs;
    ArgExprs.push_back(PragmaNameLoc);
    if (!HandlePragmaBlockLoop(&ArgExprs))
      continue;
    SourceRange Range = SourceRange(Info->PragmaName.getLocation(),
                                    Info->LastToks.empty()
                                        ? Info->PragmaName.getLocation()
                                        : Info->LastToks.back().getLocation());
    TempAttrs.addNew(PragmaNameLoc->Ident, Range, nullptr, PragmaNameLoc->Loc,
                     ArgExprs.data(), ArgExprs.size(), ParsedAttr::AS_Pragma);
  }
  // Get the next statement.
  MaybeParseCXX11Attributes(Attrs);

  StmtResult S = ParseStatementOrDeclarationAfterAttributes(
      Stmts, StmtCtx, TrailingElseLoc, Attrs);

  Attrs.takeAllFrom(TempAttrs);
  return S;
}

/// StopTk is TokenKind(r_paren, comma, colon) which stop parser for next
/// token string.
/// For an expression: potential problem for tok::colon and tok::comma
/// But for what we are parsering now, we are okay.
/// currently:
/// tok::colon is a separator for set of constant values only(level(con1:cnn2)).
/// tok::comma is a separator for variable identifiers only(private(var,var1)).
static bool ParseIntelBlockLoopToken(Preprocessor &PP, Token &Tok,
                                     Token PragmaName, Token Option,
                                     tok::TokenKind StopTK,
                                     PragmaBlockLoopInfo *Info) {
  SmallVector<Token, 1> ValueList;
  int getToken = 1;
  while (Tok.isNot(tok::eod)) {
    if (Tok.is(tok::l_paren))
      getToken++;
    else if (Tok.is(StopTK) || Tok.is(tok::r_paren)) {
      getToken--;
      if (getToken == 0)
        break;
    }

    ValueList.push_back(Tok);
    PP.Lex(Tok);
  }
  if (Tok.isNot(tok::r_paren) && Tok.isNot(StopTK)) {
    PP.Diag(Tok.getLocation(), diag::err_expected) << StopTK << tok::r_paren;
    return true;
  }

  Token EOFTok;
  EOFTok.startToken();
  EOFTok.setKind(tok::eof);
  EOFTok.setLocation(Tok.getLocation());
  ValueList.push_back(EOFTok); // Terminates expression for parsing.

  IdentifierInfo *OptionInfo = Option.getIdentifierInfo();
  if (OptionInfo->isStr("private")) {
    Info->LastToks =
        llvm::makeArrayRef(ValueList).copy(PP.getPreprocessorAllocator());
    Info->Privates.push_back(Info->LastToks);
    Info->Private = Option;
  }
  if (OptionInfo->isStr("factor")) {
    Info->LastToks = Info->Factors =
          llvm::makeArrayRef(ValueList).copy(PP.getPreprocessorAllocator());
    Info->Factor = Option;
  }
  if (OptionInfo->isStr("level")) {
    Info->LastToks =
        llvm::makeArrayRef(ValueList).copy(PP.getPreprocessorAllocator());
    Info->Levels.push_back(Info->LastToks);
    Info->Level = Option;
    // level(3) ==> level(3:3)
    if (Tok.is(tok::r_paren) && Info->Levels.size() == 1) {
      Info->Levels.push_back(Info->LastToks);
    }
  }
  Info->PragmaName = PragmaName;
  return false;
}

/// Handle the block_loop pragma
/// #pragma block_loop [ clause[, clause]...]
///
/// where clause is one of the following:
///   level (levels) :
///     levels : const1 | const1 : const2
///   factor (scalar-expr)
///   private (variable-list) :
///     variable-list (scalar-variable, scalar-variable)
void PragmaBlockLoopHandler::HandlePragma(Preprocessor &PP,
                                          PragmaIntroducer Introducer,
                                          Token &Tok) {
  // Incoming token is "block_loop"
  Token PragmaName = Tok;
  SmallVector<Token, 4> TokenList;

  if (Tok.isNot(tok::identifier)) {
    PP.Diag(Tok.getLocation(), diag::err_pragma_loop_invalid_option)
        << "block_loop";
    return;
  }
  bool HasFactor = false;
  bool HasLevel = false;
  bool HasPrivate = false;
  PragmaBlockLoopInfo *Info =
      new (PP.getPreprocessorAllocator()) PragmaBlockLoopInfo;
  do {
    Token Option = Tok;
    tok::TokenKind StopTK = tok::r_paren;
    IdentifierInfo *OptionInfo = Tok.getIdentifierInfo();
    bool OptionValid = llvm::StringSwitch<bool>(OptionInfo->getName())
                           .Case("block_loop", true)
                           .Case("level", true)
                           .Case("factor", true)
                           .Case("private", true)
                           .Default(false);
   if (!OptionValid) {
      PP.Diag(Tok.getLocation(), diag::warn_pragma_block_loop_invalid_option)
          << /*MissingOption=*/false << OptionInfo;
      return;
    }
    if (OptionInfo->isStr("factor")) {
      if (HasFactor) {
        PP.Diag(Tok.getLocation(), diag::warn_multiple_blockloop_clause) << 0;
        return;
      }
      HasFactor = true;
    }
    if (OptionInfo->isStr("level")) {
      if (HasLevel) {
        PP.Diag(Tok.getLocation(), diag::warn_multiple_blockloop_clause) << 1;
        return;
      }
      HasLevel = true;
      StopTK = tok::colon;
    }
    if (OptionInfo->isStr("private")) {
      if (HasPrivate) {
        PP.Diag(Tok.getLocation(), diag::warn_multiple_blockloop_clause) << 2;
        return;
      }
      HasPrivate = true;
      StopTK = tok::comma;
    }
    PP.Lex(Tok);
    if (!OptionInfo->isStr("block_loop")) {
      if (Tok.isNot(tok::l_paren)) {
        PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_lparen)
            << "block_loop";
        return;
      }
      PP.Lex(Tok);
      if (ParseIntelBlockLoopToken(PP, Tok, PragmaName, Option, StopTK, Info))
        return;
    }
    if ((OptionInfo->isStr("level") && Tok.is(tok::colon)) ||
        (OptionInfo->isStr("private") && Tok.is(tok::comma))) {
      do {
        PP.Lex(Tok);
        if (ParseIntelBlockLoopToken(PP, Tok, PragmaName, Option, StopTK, Info))
          return;
      } while (OptionInfo->isStr("private") && Tok.isNot(tok::r_paren));
    }
    if (Tok.is(tok::r_paren))
      PP.Lex(Tok);
    if (Tok.is(tok::comma))
      PP.Lex(Tok);
  } while (Tok.getIdentifierInfo() &&
           Tok.getIdentifierInfo()->getName() != "block_loop");

  if (Tok.isNot(tok::eod)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol)
        << "block_loop";
    return;
  }
  Info->PragmaName = PragmaName;
  Token BlockLoopTok;
  BlockLoopTok.startToken();
  BlockLoopTok.setKind(tok::annot_pragma_blockloop);
  BlockLoopTok.setLocation(PragmaName.getLocation());
  BlockLoopTok.setAnnotationEndLoc(PragmaName.getLocation());
  BlockLoopTok.setAnnotationValue(static_cast<void *>(Info));
  TokenList.push_back(BlockLoopTok);
  auto TokenArray = std::make_unique<Token[]>(TokenList.size());
  std::copy(TokenList.begin(), TokenList.end(), TokenArray.get());

  PP.EnterTokenStream(std::move(TokenArray), TokenList.size(),
                      /*DisableMacroExpansion=*/false, /*IsReinject*/ false);
}

void Parser::initializeIntelPragmaHandlers() {
  if (getLangOpts().IntelCompat) {
    // #pragma inline
    InlineHandler.reset(new PragmaInlineHandler("inline"));
    PP.AddPragmaHandler(InlineHandler.get());
    // #pragma noinline
    NoInlineHandler.reset(new PragmaInlineHandler("noinline"));
    PP.AddPragmaHandler(NoInlineHandler.get());
    // #pragma forceinline
    ForceInlineHandler.reset(new PragmaInlineHandler("forceinline"));
    PP.AddPragmaHandler(ForceInlineHandler.get());
  }
  // #pragma block_loop
  if (getLangOpts().isIntelCompat(LangOptions::PragmaBlockLoop)) {
    BlockLoopHandler.reset(new PragmaBlockLoopHandler("block_loop"));
    PP.AddPragmaHandler(BlockLoopHandler.get());
  }
}

void Parser::resetIntelPragmaHandlers() {
  // Remove the pragma handlers we installed.
  if (getLangOpts().IntelCompat) {
    // #pragma inline
    PP.RemovePragmaHandler(InlineHandler.get());
    InlineHandler.reset();
    // #pragma noinline
    PP.RemovePragmaHandler(NoInlineHandler.get());
    NoInlineHandler.reset();
    // #pragma forceinline
    PP.RemovePragmaHandler(ForceInlineHandler.get());
    ForceInlineHandler.reset();
  }
  // #pragma block_loop
  if (getLangOpts().isIntelCompat(LangOptions::PragmaBlockLoop)) {
    PP.RemovePragmaHandler(BlockLoopHandler.get());
    BlockLoopHandler.reset();
  }
}
