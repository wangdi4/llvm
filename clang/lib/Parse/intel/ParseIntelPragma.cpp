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
#include "clang/Sema/LoopHint.h"
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
  PP.EnterTokenStream(std::move(Toks), Size, DisableMacroExpansion);
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
                                     AllowedConstructsKind Allowed,
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
                     AttributeList::AS_Pragma);

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
      Stmts, Allowed, TrailingElseLoc, Attrs);

  Attrs.takeAllFrom(TempAttrs);
  return S;
}

void PragmaInlineHandler::HandlePragma(Preprocessor &PP,
                                       PragmaIntroducerKind Introducer,
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

void Parser::initializeIntelPragmaHandlers() {
#if INTEL_CUSTOMIZATION
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
#endif // INTEL_CUSTOMIZATION
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
}
