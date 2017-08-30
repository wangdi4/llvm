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

#if INTEL_SPECIFIC_CILKPLUS

// #pragma simd
void PragmaSIMDHandler::HandlePragma(Preprocessor &PP,
                                     PragmaIntroducerKind Introducer,
                                     Token &FirstTok) {
  SmallVector<Token, 16> Pragma;
  Token Tok;
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_simd);
  Tok.setLocation(FirstTok.getLocation());

  while (Tok.isNot(tok::eod)) {
    Pragma.push_back(Tok);
    PP.Lex(Tok);
  }
  SourceLocation EodLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_simd_end);
  Tok.setLocation(EodLoc);
  Pragma.push_back(Tok);

  EnterTokenS(PP, Pragma, /*DisableMacroExpansion=*/true);
}

/// \brief Handle Cilk Plus grainsize pragma.
///
/// #pragma 'cilk' 'grainsize' '=' expr new-line
///
void PragmaCilkGrainsizeHandler::HandlePragma(Preprocessor &PP,
                                              PragmaIntroducerKind Introducer,
                                              Token &FirstToken) {
  Token Tok;
  PP.Lex(Tok);
  if (Tok.isNot(tok::identifier)) {
    PP.Diag(Tok, diag::warn_pragma_expected_identifier) << "cilk";
    return;
  }

  IdentifierInfo *Grainsize = Tok.getIdentifierInfo();
  SourceLocation GrainsizeLoc = Tok.getLocation();

  if (!Grainsize->isStr("grainsize")) {
    PP.Diag(Tok, diag::err_cilk_for_expect_grainsize);
    return;
  }

  PP.Lex(Tok);
  if (Tok.isNot(tok::equal)) {
    PP.Diag(Tok, diag::err_cilk_for_expect_assign);
    return;
  }

  // Cache tokens after '=' and store them back to the token stream.
  SmallVector<Token, 5> CachedToks;
  while (true) {
    PP.Lex(Tok);
    if (Tok.is(tok::eod))
      break;
    CachedToks.push_back(Tok);
  }

  llvm::BumpPtrAllocator &Allocator = PP.getPreprocessorAllocator();
  unsigned Size = CachedToks.size();

  Token *Toks = (Token *) Allocator.Allocate(sizeof(Token) * (Size + 2),
                                             alignof(Token));
  Token &GsBeginTok = Toks[0];
  GsBeginTok.startToken();
  GsBeginTok.setKind(tok::annot_pragma_cilk_grainsize_begin);
  GsBeginTok.setLocation(FirstToken.getLocation());

  SourceLocation EndLoc = Size ? CachedToks.back().getLocation()
                               : GrainsizeLoc;

  Token &GsEndTok = Toks[Size + 1];
  GsEndTok.startToken();
  GsEndTok.setKind(tok::annot_pragma_cilk_grainsize_end);
  GsEndTok.setLocation(EndLoc);

  for (unsigned i = 0; i < Size; ++i)
    Toks[i + 1] = CachedToks[i];

  PP.EnterTokenStream(ArrayRef<Token> (Toks, Size + 2), /*DisableMacroExpansion=*/true);
}
#endif // INTEL_SPECIFIC_CILKPLUS

#ifdef INTEL_SPECIFIC_IL0_BACKEND

static void EnterOneTokenS(Preprocessor &PP, Token &FirstTok, tok::TokenKind K) {
  Token T;
  T.startToken();
  T.setKind(K);
  T.setLocation(FirstTok.getLocation());
  T.setAnnotationValue(static_cast<void*>(0));
  Token *Tp = &T;

  PP.EnterTokenStream(ArrayRef<Token> (Tp, 1), false);
}

void Parser::DiscardBeforeEndOfDirective() {
  while (Tok.isNot(tok::annot_pragma_end)) {
    PP.Lex(Tok);
  }
}

void Parser::DiscardUntilEndOfDirective() {
  DiscardBeforeEndOfDirective();
  PP.Lex(Tok);
}

// #pragma ivdep

StmtResult Parser::HandlePragmaIvdep() {
  assert(Tok.is(tok::annot_pragma_ivdep));
  SourceLocation IvdepLoc = ConsumeToken();
  Sema::IntelPragmaIvdepOption Opt = Sema::IntelPragmaIvdepOptionNone;

  if (Tok.isNot(tok::annot_pragma_end)) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    if(II && II->isStr("loop")) {
      Opt = Sema::IntelPragmaIvdepOptionLoop;
      PP.Lex(Tok);
    }
  }

  if (Tok.isNot(tok::annot_pragma_end)) {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_malformed);
  }
  DiscardUntilEndOfDirective();

  return (Actions.ActOnPragmaOptionsIvdep(IvdepLoc, Opt));
}

void Parser::HandlePragmaIvdepDecl() {
  assert(Tok.is(tok::annot_pragma_ivdep));
  SourceLocation IvdepLoc = ConsumeToken();

  DiscardUntilEndOfDirective();

  Diag(IvdepLoc, diag::x_warn_intel_pragma_statement_precede)<<IvdepLoc;
}

void PragmaIvdepHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation IvdepLoc = FirstTok.getLocation();
  
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_ivdep);
  Tok.setLocation(IvdepLoc);
  Tok.setAnnotationValue(static_cast<void*>(0));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  IvdepLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(IvdepLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

// #pragma novector

StmtResult Parser::HandlePragmaNoVector() {
  assert(Tok.is(tok::annot_pragma_novector));
  SourceLocation NoVectorLoc = ConsumeToken();
  return (Actions.ActOnPragmaOptionsNoVector(NoVectorLoc));
}

void Parser::HandlePragmaNoVectorDecl() {
  assert(Tok.is(tok::annot_pragma_novector));
  SourceLocation NoVectorLoc = ConsumeToken();

  Diag(NoVectorLoc, diag::x_warn_intel_pragma_statement_precede)<<NoVectorLoc;
}

void PragmaNoVectorHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  // ignore everything till the end of line
  PP.DiscardUntilEndOfDirective();
  EnterOneTokenS(PP, FirstTok, tok::annot_pragma_novector);
}

// #pragma distribute_point

StmtResult Parser::HandlePragmaDistribute() {
  assert(Tok.is(tok::annot_pragma_distribute_point));
  SourceLocation DistributeLoc = ConsumeToken();
  return (Actions.ActOnPragmaOptionsDistribute(DistributeLoc));
}

void Parser::HandlePragmaDistributeDecl() {
  assert(Tok.is(tok::annot_pragma_distribute_point));
  SourceLocation DistributeLoc = ConsumeToken();
  Diag(DistributeLoc, diag::x_error_intel_pragma_statement_precede)<<DistributeLoc;
}

void PragmaDistributeHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  // ignore everything till the end of line
  PP.DiscardUntilEndOfDirective();
  EnterOneTokenS(PP, FirstTok, tok::annot_pragma_distribute_point);
}

void PragmaDistributeHandler1::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;

  // ignore everything till the end of line
  PP.Lex(Tok);
  if (Tok.isAnyIdentifier()) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    if (!II || !II->isStr("point")) {
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"'point'"<<0;
      PP.DiscardUntilEndOfDirective();
      return;
    }
    PP.Lex(Tok);
  }

  EnterOneTokenS(PP, FirstTok, tok::annot_pragma_distribute_point);
}

// #pragma inline [recursive]
// #pragma forceinline [recursive]
// #pragma noinline

StmtResult Parser::HandlePragmaInline() {
  assert(Tok.is(tok::annot_pragma_inline));
  Sema::IntelPragmaInlineOption Opt = Sema::IntelPragmaInlineOptionNone;
  const Sema::IntelPragmaInlineKind *KindPtr = 
    static_cast<Sema::IntelPragmaInlineKind *>(Tok.getAnnotationValue());
  const Sema::IntelPragmaInlineKind Kind = *KindPtr;
  SourceLocation InlineLoc = ConsumeToken();

  if (Tok.isNot(tok::annot_pragma_end) && Kind != Sema::IntelPragmaNoInline) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    if(II && II->isStr("recursive")) {
      Opt = Sema::IntelPragmaInlineOptionRecursive;
      PP.Lex(Tok);
    }
  }

  if (Tok.isNot(tok::annot_pragma_end)) {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_extra_text);
  }
  DiscardUntilEndOfDirective();

  delete KindPtr;

  return (Actions.ActOnPragmaOptionsInline(InlineLoc, Kind, Opt));
}

void Parser::HandlePragmaInlineDecl() {
  assert(Tok.is(tok::annot_pragma_inline));
  const Sema::IntelPragmaInlineKind *KindPtr = 
    static_cast<Sema::IntelPragmaInlineKind *>(Tok.getAnnotationValue());
  SourceLocation InlineLoc = ConsumeToken();

  delete KindPtr;
  DiscardUntilEndOfDirective();

  Diag(InlineLoc, diag::x_error_intel_pragma_statement_precede)<<InlineLoc;
}

void PragmaInlineHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation InlineLoc = FirstTok.getLocation();
  Sema::IntelPragmaInlineKind *KindPtr = new Sema::IntelPragmaInlineKind;

  *KindPtr = Sema::IntelPragmaSimpleInline;
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_inline);
  Tok.setLocation(InlineLoc);
  Tok.setAnnotationValue(static_cast<void*>(KindPtr));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
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

void PragmaForceInlineHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation InlineLoc = FirstTok.getLocation();
  Sema::IntelPragmaInlineKind *KindPtr = new Sema::IntelPragmaInlineKind;

  *KindPtr = Sema::IntelPragmaForceInline;
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_inline);
  Tok.setLocation(InlineLoc);
  Tok.setAnnotationValue(static_cast<void*>(KindPtr));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
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

void PragmaNoInlineHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation InlineLoc = FirstTok.getLocation();
  Sema::IntelPragmaInlineKind *KindPtr = new Sema::IntelPragmaInlineKind;

  *KindPtr = Sema::IntelPragmaNoInline;
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_inline);
  Tok.setLocation(InlineLoc);
  Tok.setAnnotationValue(static_cast<void*>(KindPtr));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
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

// #pragma loop_count (n)
// #pragma loop_count =n
// #pragma loop_count (n1, n2, ...)
// #pragma loop_count =n1,n2,...
// #pragma loop_count min(n1),avg(n2),max(n3)
// #pragma loop_count min=n1, avg=n2, max=n3

StmtResult Parser::HandlePragmaLoopCount() {
  assert(Tok.is(tok::annot_pragma_loop_count));
  SourceLocation LoopCountLoc = ConsumeToken();
  bool MinDone = false;
  bool MaxDone = false;
  bool AvgDone = false;
  SmallVector<ExprResult, 4> MinAvgMax(3, ExprError());
  SmallVector<ExprResult, 4> Regular;

  // Lex the left '(' or '=', if any, and just ignore them
  if (Tok.is(tok::l_paren) || Tok.is(tok::equal)) {
    PP.Lex(Tok);
  }

  if (Tok.isNot(tok::annot_pragma_end)) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    // Parse min, avg or max
    size_t Ind = 3;
    if (II && II->isStr("min")) Ind = 0;
    if (II && II->isStr("avg")) Ind = 1;
    if (II && II->isStr("max")) Ind = 2;
    if (Ind < 3) {
      // min, avg, max in action
      while (Tok.isNot(tok::annot_pragma_end)) {
        if((Ind == 0 && MinDone) || (Ind == 1 && AvgDone) || (Ind == 2 && MaxDone)) {
          // Error - parsed already
          PP.Diag(Tok.getLocation(), diag::x_error_intel_pragma_loop_count);
          DiscardUntilEndOfDirective();
          return (StmtError());
        } else if (Ind == 3) {
          // Warning - extra text
          break;
        }
        PP.Lex(Tok);
        // Lex the left '(' or '=', if any, and just ignore them
        if (Tok.is(tok::l_paren) || Tok.is(tok::equal)) {
          PP.Lex(Tok);
        }
        // Parse Constant Integer Expression
        MinAvgMax[Ind] = ParseConstantExpression();
        if(!MinAvgMax[Ind].isUsable()) {
          // Error is found
          DiscardUntilEndOfDirective();
          return (StmtError());
        }
        MinDone = MinDone || (Ind == 0);
        AvgDone = AvgDone || (Ind == 1);
        MaxDone = MaxDone || (Ind == 2);
        // Lex the right ')', if any, and just ignore
        if (Tok.is(tok::r_paren)) {
          PP.Lex(Tok);
        }
        // Lex the ',', if any, and just ignore
        if (Tok.is(tok::comma)) {
          PP.Lex(Tok);
        }

        Ind = 3;
        if (Tok.isNot(tok::annot_pragma_end)) {
          II = Tok.getIdentifierInfo();
          if (II && II->isStr("min")) Ind = 0;
          if (II && II->isStr("avg")) Ind = 1;
          if (II && II->isStr("max")) Ind = 2;
        }
      }
    }
    else {
      // Integer sequence is in action
      while (Tok.isNot(tok::annot_pragma_end)) {
        // Parse Constant Integer Expression
        Regular.push_back(ParseConstantExpression());
        if(!Regular.back().isUsable()) {
          DiscardUntilEndOfDirective();
          return (StmtError());
        }
        // Lex the ',', if any, and just ignore
        if (Tok.is(tok::comma)) {
          PP.Lex(Tok);
        }
        // Lex the right ')', if any, and just ignore
        if (Tok.is(tok::r_paren)) {
          PP.Lex(Tok);
          break;
        }
      }
    }
  }

  if (Tok.isNot(tok::annot_pragma_end)) {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_extra_text);
  }
  DiscardUntilEndOfDirective();

  return (Actions.ActOnPragmaOptionsLoopCount(LoopCountLoc, MinAvgMax, Regular));
}

void Parser::HandlePragmaLoopCountDecl() {
  assert(Tok.is(tok::annot_pragma_loop_count));
  SourceLocation LoopCountLoc = ConsumeToken();

  DiscardUntilEndOfDirective();

  Diag(LoopCountLoc, diag::x_error_intel_pragma_statement_precede)<<LoopCountLoc;
}

void PragmaLoopCountHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation LoopCountLoc = FirstTok.getLocation();

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_loop_count);
  Tok.setLocation(LoopCountLoc);
  Tok.setAnnotationValue(static_cast<void*>(0));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  LoopCountLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(LoopCountLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

void PragmaLoopCountHandler1::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation LoopCountLoc = FirstTok.getLocation();

  PP.Lex(Tok);
  if (Tok.isAnyIdentifier()) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    if (!II || !II->isStr("count")) {
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"'count'"<<0;
      PP.DiscardUntilEndOfDirective();
      return;
    }
  }

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_loop_count);
  Tok.setLocation(LoopCountLoc);
  Tok.setAnnotationValue(static_cast<void*>(0));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  LoopCountLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(LoopCountLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

// #pragma optimize ("string", on|off)

StmtResult Parser::HandlePragmaOptimize() {
  assert(Tok.is(tok::annot_pragma_optimize));
  SourceLocation OptimizeLoc = ConsumeToken();
  Sema::IntelPragmaOptimizeOption Opt = Sema::IntelPragmaOptimizeOptionOn;
  bool Warned = false;

  // Lex the left '(', if any, and just ignore
  if (Tok.is(tok::l_paren)) {
    PP.Lex(Tok);
  }

  if (Tok.isNot(tok::string_literal)) {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_string);
    DiscardBeforeEndOfDirective();
    Warned = true;
  }
  else if (Tok.isNot(tok::annot_pragma_end)) {
    PP.Lex(Tok);
  }
  // Lex the ',', if any, and just ignore
  if (Tok.is(tok::comma)) {
    PP.Lex(Tok);
  }
  if (Tok.isNot(tok::annot_pragma_end)) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    if (!II || !(II->isStr("on") || II->isStr("off"))) {
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_on_off_expected);
      DiscardBeforeEndOfDirective();
    } else {
      Opt = (II->isStr("on")) ? Sema::IntelPragmaOptimizeOptionOn : Sema::IntelPragmaOptimizeOptionOff;
      PP.Lex(Tok);
    }
  } 
  else if (!Warned) {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_on_off_expected);
  }
  // Lex the ')', if any, and just ignore
  if (Tok.is(tok::r_paren)) {
    PP.Lex(Tok);
  }
  if (Tok.isNot(tok::annot_pragma_end)) {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_extra_text);
  }
  DiscardUntilEndOfDirective();

  return (Actions.ActOnPragmaOptionsOptimize(OptimizeLoc, Opt));
}

void Parser::HandlePragmaOptimizeDecl() {
  assert(Tok.is(tok::annot_pragma_optimize));
  StmtResult Res = HandlePragmaOptimize();

  if (Res.get() && !(static_cast<PragmaStmt *>(Res.get()))->isNullOp())
    Actions.ActOnPragmaOptionsOptimize(static_cast<PragmaStmt *>(Res.get()));
}

void PragmaIntelOptimizeHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation OptimizeLoc = FirstTok.getLocation();

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_optimize);
  Tok.setLocation(OptimizeLoc);
  Tok.setAnnotationValue(static_cast<void*>(0));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  OptimizeLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(OptimizeLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

// #pragma [GCC|intel] optimization_level 0-3|reset

void Parser::HandlePragmaOptimizationLevel() {
  assert(Tok.is(tok::annot_pragma_optimization_level));
  SourceLocation OptLevelLoc = ConsumeToken();

  DiscardUntilEndOfDirective();

  Diag(OptLevelLoc, diag::x_error_intel_pragma_declaration_precede)<<OptLevelLoc;
}

void Parser::HandlePragmaOptimizationLevelDecl() {
  assert(Tok.is(tok::annot_pragma_optimization_level));
  bool IsIntelPragma = (Tok.getAnnotationValue() == 0);
  SourceLocation OptLevelLoc = ConsumeToken();
  Token Tmp = Tok;

  // Lex the left '(', if any, and just ignore
  if (Tok.is(tok::l_paren)) {
    PP.Lex(Tok);
    Tmp = Tok;
  }

  if (Tok.isNot(tok::numeric_constant) && !(!IsIntelPragma && Tok.getIdentifierInfo() &&
    Tok.getIdentifierInfo()->isStr("reset"))) {
    PP.Diag(Tok.getLocation(), 
     IsIntelPragma ? diag::x_warn_intel_pragma_integer_const : diag::x_warn_intel_pragma_reset_integer_const);
    DiscardUntilEndOfDirective();
    return;
  }
  else {
    PP.Lex(Tok);
  }

  // Lex the left ')', if any, and just ignore
  if (Tok.is(tok::r_paren)) {
    PP.Lex(Tok);
  }

  if (Tok.isNot(tok::annot_pragma_end)) {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_extra_text);
  }
  DiscardUntilEndOfDirective();

  Actions.ActOnPragmaOptionsOptimizationLevel(OptLevelLoc, Tmp, IsIntelPragma);
}

void PragmaOptimizationLevelHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation OptLevelLoc = FirstTok.getLocation();

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_optimization_level);
  Tok.setLocation(OptLevelLoc);
  // Annotation is 0 if Intel pragma and !0 if GCC
  Tok.setAnnotationValue(static_cast<void*>(IsIntelPragma ? 0 : this));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  OptLevelLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(OptLevelLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

// #pragma noparallel

StmtResult Parser::HandlePragmaNoParallel() {
  assert(Tok.is(tok::annot_pragma_noparallel));
  SourceLocation NoParallelLoc = ConsumeToken();
  return (Actions.ActOnPragmaOptionsNoParallel(NoParallelLoc));
}

void Parser::HandlePragmaNoParallelDecl() {
  assert(Tok.is(tok::annot_pragma_noparallel));
  SourceLocation NoParallelLoc = ConsumeToken();

  Diag(NoParallelLoc, diag::x_warn_intel_pragma_statement_precede)<<NoParallelLoc;
}

void PragmaNoParallelHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  // ignore everything till the end of line
  PP.DiscardUntilEndOfDirective();
  EnterOneTokenS(PP, FirstTok, tok::annot_pragma_noparallel);
}

// #pragma nounroll
// #pragma unroll ([n])

StmtResult Parser::HandlePragmaUnroll() {
  assert(Tok.is(tok::annot_pragma_unroll));
  const Sema::IntelPragmaUnrollKind *KindPtr = 
    static_cast<Sema::IntelPragmaUnrollKind *>(Tok.getAnnotationValue());
  const Sema::IntelPragmaUnrollKind Kind = *KindPtr;
  SourceLocation UnrollLoc = ConsumeToken();
  ExprResult Opt;

  // Lex the left '('
  if (Kind == Sema::IntelPragmaSimpleUnroll && Tok.isNot(tok::annot_pragma_end)) {
    if (Tok.isNot(tok::l_paren)) {
      Diag(UnrollLoc, diag::warn_pragma_expected_lparen)<<"unroll";
      DiscardBeforeEndOfDirective();
    }
    else {
      PP.Lex(Tok);
      if (Tok.isNot(tok::annot_pragma_end)) {
        Opt = ParseConstantExpression();
      }
      // Lex the right ')' if any
      if (Tok.is(tok::r_paren)) {
        PP.Lex(Tok);
      }
    }
  }

  if (Tok.isNot(tok::annot_pragma_end)) {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_extra_text);
  }
  DiscardUntilEndOfDirective();

  delete KindPtr;

  return (Actions.ActOnPragmaOptionsUnroll(UnrollLoc, Kind, Opt));
}

void Parser::HandlePragmaUnrollDecl() {
  assert(Tok.is(tok::annot_pragma_unroll));
  const Sema::IntelPragmaUnrollKind *KindPtr = 
    static_cast<Sema::IntelPragmaUnrollKind *>(Tok.getAnnotationValue());
  SourceLocation UnrollLoc = ConsumeToken();

  delete KindPtr;
  DiscardUntilEndOfDirective();

  Diag(UnrollLoc, diag::x_error_intel_pragma_statement_precede)<<UnrollLoc;
}

void PragmaUnrollHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation UnrollLoc = FirstTok.getLocation();
  Sema::IntelPragmaUnrollKind *KindPtr = new Sema::IntelPragmaUnrollKind;

  *KindPtr = Sema::IntelPragmaSimpleUnroll;
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_unroll);
  Tok.setLocation(UnrollLoc);
  Tok.setAnnotationValue(static_cast<void*>(KindPtr));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  UnrollLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(UnrollLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

void PragmaNoUnrollHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation UnrollLoc = FirstTok.getLocation();
  Sema::IntelPragmaUnrollKind *KindPtr = new Sema::IntelPragmaUnrollKind;

  *KindPtr = Sema::IntelPragmaNoUnroll;
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_unroll);
  Tok.setLocation(UnrollLoc);
  Tok.setAnnotationValue(static_cast<void*>(KindPtr));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  UnrollLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(UnrollLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

// #pragma nounroll_and_jam
// #pragma unroll_and_jam ([n])

StmtResult Parser::HandlePragmaUnrollAndJam() {
  assert(Tok.is(tok::annot_pragma_unroll_and_jam));
  const Sema::IntelPragmaUnrollAndJamKind *KindPtr = 
    static_cast<Sema::IntelPragmaUnrollAndJamKind *>(Tok.getAnnotationValue());
  const Sema::IntelPragmaUnrollAndJamKind Kind = *KindPtr;
  SourceLocation UnrollAndJamLoc = ConsumeToken();
  ExprResult Opt;

  // Lex the left '('
  if (Kind == Sema::IntelPragmaSimpleUnrollAndJam && Tok.isNot(tok::annot_pragma_end)) {
    if (Tok.isNot(tok::l_paren)) {
      Diag(UnrollAndJamLoc, diag::warn_pragma_expected_lparen)<<"unroll_and_jam";
      DiscardBeforeEndOfDirective();
    }
    else {
      PP.Lex(Tok);
      if (Tok.isNot(tok::annot_pragma_end)) {
        Opt = ParseConstantExpression();
      }
      // Lex the right ')' if any
      if (Tok.is(tok::r_paren)) {
        PP.Lex(Tok);
      }
    }
  }

  if (Tok.isNot(tok::annot_pragma_end)) {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_extra_text);
  }
  DiscardUntilEndOfDirective();

  delete KindPtr;

  return (Actions.ActOnPragmaOptionsUnrollAndJam(UnrollAndJamLoc, Kind, Opt));
}

void Parser::HandlePragmaUnrollAndJamDecl() {
  assert(Tok.is(tok::annot_pragma_unroll_and_jam));
  const Sema::IntelPragmaUnrollAndJamKind *KindPtr = 
    static_cast<Sema::IntelPragmaUnrollAndJamKind *>(Tok.getAnnotationValue());
  SourceLocation UnrollAndJamLoc = ConsumeToken();

  delete KindPtr;
  DiscardUntilEndOfDirective();

  Diag(UnrollAndJamLoc, diag::x_error_intel_pragma_statement_precede)<<UnrollAndJamLoc;
}

void PragmaUnrollAndJamHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation UnrollAndJamLoc = FirstTok.getLocation();
  Sema::IntelPragmaUnrollAndJamKind *KindPtr = new Sema::IntelPragmaUnrollAndJamKind;

  *KindPtr = Sema::IntelPragmaSimpleUnrollAndJam;
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_unroll_and_jam);
  Tok.setLocation(UnrollAndJamLoc);
  Tok.setAnnotationValue(static_cast<void*>(KindPtr));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  UnrollAndJamLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(UnrollAndJamLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

void PragmaNoUnrollAndJamHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation UnrollAndJamLoc = FirstTok.getLocation();
  Sema::IntelPragmaUnrollAndJamKind *KindPtr = new Sema::IntelPragmaUnrollAndJamKind;
  
  *KindPtr = Sema::IntelPragmaNoUnrollAndJam;
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_unroll_and_jam);
  Tok.setLocation(UnrollAndJamLoc);
  Tok.setAnnotationValue(static_cast<void*>(KindPtr));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  UnrollAndJamLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(UnrollAndJamLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

// #pragma nofusion

StmtResult Parser::HandlePragmaNoFusion() {
  assert(Tok.is(tok::annot_pragma_nofusion));
  SourceLocation NoFusionLoc = ConsumeToken();
  return (Actions.ActOnPragmaOptionsNoFusion(NoFusionLoc));
}

void Parser::HandlePragmaNoFusionDecl() {
  assert(Tok.is(tok::annot_pragma_nofusion));
  SourceLocation NoFusionLoc = ConsumeToken();

  Diag(NoFusionLoc, diag::x_error_intel_pragma_statement_precede)<<NoFusionLoc;
}

void PragmaNoFusionHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  // ignore everything till the end of line
  PP.DiscardUntilEndOfDirective();
  EnterOneTokenS(PP, FirstTok, tok::annot_pragma_nofusion);
}

// #pragma ident
void PragmaIdentHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  // Read the string argument.
  Token StrTok;
  PP.Lex(StrTok);

  // If the token kind isn't a string, it's a malformed directive.
  if (StrTok.isNot(tok::string_literal) &&
        StrTok.isNot(tok::wide_string_literal)) {
     PP.Diag(StrTok, diag::err_expected_string_literal);
     if (StrTok.isNot(tok::eod))
        PP.DiscardUntilEndOfDirective();
     return;
  }

  if (StrTok.hasUDSuffix()) {
     PP.Diag(StrTok, diag::err_invalid_string_udl);
     PP.DiscardUntilEndOfDirective();
     return;
  }

  // Verify that there is nothing after the string, other than EOD.
  PP.CheckEndOfDirective("pragma ident");

  if (PP.getPPCallbacks()) {
     bool Invalid = false;
     std::string Str = PP.getSpelling(StrTok, &Invalid);
     if (!Invalid)
       PP.getPPCallbacks()->Ident(FirstTok.getLocation(), Str);
  }
}

// #pragma vector

StmtResult Parser::HandlePragmaVector() {
  assert(Tok.is(tok::annot_pragma_vector));
  SourceLocation VectorLoc = ConsumeToken();
  int Opt = 0;
  SmallVector<ExprResult, 4> Exprs;

  while (Tok.isNot(tok::annot_pragma_end)) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    if (II) {
      if (II->isStr("always")) {
        Opt |= Sema::IntelPragmaVectorAlways;
        PP.Lex(Tok);
      }
      else if (II->isStr("assert")) {
        Opt |= Sema::IntelPragmaVectorAssert;
        PP.Lex(Tok);
        if (!(Opt & Sema::IntelPragmaVectorAlways)) {
          PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_assert_before_always);
          DiscardUntilEndOfDirective();
          return (StmtEmpty());
        }
      }
      else if (II->isStr("aligned")) {
        Opt |= Sema::IntelPragmaVectorAligned;
        PP.Lex(Tok);
        if (Opt & Sema::IntelPragmaVectorUnAligned) {
          PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_qualifier_conflict);
          DiscardUntilEndOfDirective();
          return (StmtEmpty());
        }
      }
      else if (II->isStr("unaligned")) {
        Opt |= Sema::IntelPragmaVectorUnAligned;
        PP.Lex(Tok);
        if (Opt & Sema::IntelPragmaVectorAligned) {
          PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_qualifier_conflict);
          DiscardUntilEndOfDirective();
          return (StmtEmpty());
        }
      }
      else if (II->isStr("mask_readwrite")) {
        Opt |= Sema::IntelPragmaVectorMaskReadWrite;
        PP.Lex(Tok);
        if (Opt & Sema::IntelPragmaVectorNoMaskReadWrite) {
          PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_qualifier_conflict);
          DiscardUntilEndOfDirective();
          return (StmtEmpty());
        }
      }
      else if (II->isStr("nomask_readwrite")) {
        Opt |= Sema::IntelPragmaVectorNoMaskReadWrite;
        PP.Lex(Tok);
        if (Opt & Sema::IntelPragmaVectorMaskReadWrite) {
          PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_qualifier_conflict);
          DiscardUntilEndOfDirective();
          return (StmtEmpty());
        }
      }
      else if (II->isStr("vecremainder")) {
        Opt |= Sema::IntelPragmaVectorVecRemainder;
        PP.Lex(Tok);
        if (Opt & Sema::IntelPragmaVectorNoVecRemainder) {
          PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_qualifier_conflict);
          DiscardUntilEndOfDirective();
          return (StmtEmpty());
        }
      }
      else if (II->isStr("novecremainder")) {
        Opt |= Sema::IntelPragmaVectorNoVecRemainder;
        PP.Lex(Tok);
        if (Opt & Sema::IntelPragmaVectorVecRemainder) {
          PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_qualifier_conflict);
          DiscardUntilEndOfDirective();
          return (StmtEmpty());
        }
      }
      else if (II->isStr("temporal")) {
        Opt |= Sema::IntelPragmaVectorTemporal;
        PP.Lex(Tok);
        if (Opt & Sema::IntelPragmaVectorNonTemporal) {
          PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_qualifier_conflict);
          DiscardUntilEndOfDirective();
          return (StmtEmpty());
        }
      }
      else if (II->isStr("nontemporal")) {
        Opt |= Sema::IntelPragmaVectorNonTemporal;
        PP.Lex(Tok);
        if (Opt & Sema::IntelPragmaVectorTemporal) {
          PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_qualifier_conflict);
          DiscardUntilEndOfDirective();
          return (StmtEmpty());
        }
        if (Tok.is(tok::l_paren)) {
          PP.Lex(Tok);
          while (Tok.isNot(tok::annot_pragma_end)) {
            // Parse expr
            //Decl::SetUseReferencedInExpr(false);
            auto Res =
                Actions.CorrectDelayedTyposInExpr(ParseAssignmentExpression());
            //Decl::SetUseReferencedInExpr(true);
            if(!Res.isUsable()) {
              // Error is found
              DiscardUntilEndOfDirective();
              return (StmtError());
            }
            Exprs.push_back(Res);
            // Parse ',', if any
            if (Tok.is(tok::comma)) {
              PP.Lex(Tok);
            }
            if(Tok.is(tok::r_paren)) {
              PP.Lex(Tok);
              break;
            }
          }
        }
      }
      else {
        PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_malformed);
        DiscardUntilEndOfDirective();
        return (StmtEmpty());
      }
    }
    else {
      break;
    }
  }
  if (Tok.isNot(tok::annot_pragma_end)) {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_extra_text);
  }
  DiscardUntilEndOfDirective();

  return (Actions.ActOnPragmaOptionsVector(getCurScope(), VectorLoc, Opt, Exprs));
}

void Parser::HandlePragmaVectorDecl() {
  assert(Tok.is(tok::annot_pragma_vector));
  SourceLocation VectorLoc = ConsumeToken();

  DiscardUntilEndOfDirective();

  Diag(VectorLoc, diag::x_warn_intel_pragma_statement_precede)<<VectorLoc;
}

void PragmaVectorHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SourceLocation VectorLoc = FirstTok.getLocation();
  SmallVector<Token, 4> Tokens;

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_vector);
  Tok.setLocation(VectorLoc);
  Tok.setAnnotationValue(static_cast<void*>(0));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  VectorLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(VectorLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

// #pragma intel optimization_parameter target_arch=<CPU>

void Parser::HandlePragmaOptimizationParameter() {
  assert(Tok.is(tok::annot_pragma_optimization_parameter));
  SourceLocation OptParamLoc = ConsumeToken();

  DiscardUntilEndOfDirective();

  Diag(OptParamLoc, diag::x_error_intel_pragma_declaration_precede)<<OptParamLoc;
}

void Parser::HandlePragmaOptimizationParameterDecl() {
  assert(Tok.is(tok::annot_pragma_optimization_parameter));
  SourceLocation OptParamLoc = ConsumeToken();

  if (!Tok.isAnnotation() && Tok.getIdentifierInfo() && Tok.getIdentifierInfo()->isStr("target_arch")) {
    PP.Lex(Tok);
    if (Tok.is(tok::equal)) {
      PP.Lex(Tok);
    }
    std::string SCPU;
    while (Tok.isNot(tok::annot_pragma_end)) {
      SCPU += PP.getSpelling(Tok);
      PP.Lex(Tok);
    }
    if (!SCPU.empty()) {
      StringRef CPU(SCPU);
      Actions.ActOnPragmaOptionsOptimizationParameter(OptParamLoc, SCPU);
    }
  }

  DiscardUntilEndOfDirective();

}

void PragmaOptimizationParameterHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation OptParamLoc = FirstTok.getLocation();

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_optimization_parameter);
  Tok.setLocation(OptParamLoc);
  Tok.setAnnotationValue(static_cast<void*>(0));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  OptParamLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(OptParamLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

// #pragma parallel

StmtResult Parser::HandlePragmaParallel() {
  assert(Tok.is(tok::annot_pragma_parallel));
  SourceLocation ParallelLoc = ConsumeToken();
  int Opt = Sema::IntelPragmaParallelUnknown;
  ExprResult NumThreads;
  ExprResult Collapse;
  SmallVector<ExprResult, 4> Private;
  SmallVector<ExprResult, 4> SizePrivate;
  SmallVector<ExprResult, 4> LastPrivate;
  SmallVector<ExprResult, 4> SizeLastPrivate;
  SmallVector<ExprResult, 4> FirstPrivate;
  SmallVector<ExprResult, 4> SizeFirstPrivate;

  while (Tok.isNot(tok::annot_pragma_end)) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    if (II) {
      SmallVector<ExprResult, 4> *Vars;
      SmallVector<ExprResult, 4> *SizeVars;
      if (II->isStr("always")) {
        Opt |= Sema::IntelPragmaParallelAlways;
        PP.Lex(Tok);
        continue;
      }
      else if (II->isStr("assert")) {
        if (!(Opt & Sema::IntelPragmaParallelAlways)) {
          PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_assert_before_always);
          DiscardUntilEndOfDirective();
          return (StmtEmpty());
        }
        Opt |= Sema::IntelPragmaParallelAssert;
        PP.Lex(Tok);
        continue;
      }
      else if (II->isStr("collapse")) {
        Opt |= Sema::IntelPragmaParallelCollapse;
        PP.Lex(Tok);
        Collapse = ParseConstantExpression();
        if (!Collapse.isUsable()) {
          return (StmtError());
        }
        continue;
      }
      else if (II->isStr("num_threads")) {
        Opt |= Sema::IntelPragmaParallelNumThreads;
        PP.Lex(Tok);
        NumThreads = ParseConstantExpression();
        if (!NumThreads.isUsable()) {
          return (StmtError());
        }
        continue;
      }
      else if (Tok.is(tok::kw_private) || II->isStr("private")) {//II->isStr("private")) {
        Vars = &Private;
        SizeVars = &SizePrivate;
      }
      else if (II->isStr("lastprivate")) {
        Vars = &LastPrivate;
        SizeVars = &SizeLastPrivate;
      }
      else if (II->isStr("firstprivate")) {
        Vars = &FirstPrivate;
        SizeVars = &SizeFirstPrivate;
      }
      else {
        PP.Diag(Tok.getLocation(), diag::x_error_intel_pragma_invalid_parallel_pragma);
        DiscardUntilEndOfDirective();
        return (StmtError());
      }
      Opt |= Sema::IntelPragmaParallelVars;
      PP.Lex(Tok);
      if (Tok.is(tok::l_paren)) {
        PP.Lex(Tok);
      }
      else {
        PP.Diag(Tok.getLocation(), diag::x_error_intel_pragma_invalid_parallel_pragma);
        DiscardUntilEndOfDirective();
        return (StmtError());
      }
      while (Tok.isNot(tok::annot_pragma_end) && Tok.isNot(tok::r_paren)) {
        // Parse expr
        //Decl::SetUseReferencedInExpr(false);
        auto Res =
            Actions.CorrectDelayedTyposInExpr(ParseAssignmentExpression());
        //Decl::SetUseReferencedInExpr(true);
        //Res.get()->dumpAll();
        if(!Res.isUsable()) {
          // Error is found
          DiscardUntilEndOfDirective();
          return (StmtError());
        }
        Vars->push_back(Res);
        if (Tok.is(tok::colon)) {
          PP.Lex(Tok);
          Res = ParseAssignmentExpression();
        }
        else {
          Res = ExprEmpty();
        }
        SizeVars->push_back(Res);
        // Parse ',', if any
        if (Tok.is(tok::comma)) {
          PP.Lex(Tok);
        }
      }
      if (Tok.is(tok::r_paren)) {
        PP.Lex(Tok);
      }
      else {
        PP.Diag(Tok.getLocation(), diag::x_error_intel_pragma_invalid_parallel_pragma);
        DiscardUntilEndOfDirective();
        return (StmtError());
      }
      // Parse ',', if any
      if (Tok.is(tok::comma)) {
        PP.Lex(Tok);
        II = Tok.getIdentifierInfo();
      }
    }
    else {
      PP.Diag(Tok.getLocation(), diag::x_error_intel_pragma_invalid_parallel_pragma);
      DiscardUntilEndOfDirective();
      return (StmtError());
    }
  }

  if (Tok.isNot(tok::annot_pragma_end)) {
    PP.Diag(Tok.getLocation(), diag::x_error_intel_pragma_invalid_parallel_pragma);
  }
  DiscardUntilEndOfDirective();

  return (Actions.ActOnPragmaOptionsParallel(getCurScope(), ParallelLoc, Opt, Private, SizePrivate, 
    LastPrivate, SizeLastPrivate, FirstPrivate, SizeFirstPrivate, Collapse, NumThreads));
}

void Parser::HandlePragmaParallelDecl() {
  assert(Tok.is(tok::annot_pragma_parallel));
  SourceLocation ParallelLoc = ConsumeToken();

  DiscardUntilEndOfDirective();

  Diag(ParallelLoc, diag::x_warn_intel_pragma_statement_precede)<<ParallelLoc;
}

void PragmaParallelHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SourceLocation ParallelLoc = FirstTok.getLocation();
  SmallVector<Token, 4> Tokens;

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_parallel);
  Tok.setLocation(ParallelLoc);
  Tok.setAnnotationValue(static_cast<void*>(0));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  ParallelLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(ParallelLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

// #pragma alloc_section (var1, var2, ..., "string")

StmtResult Parser::HandlePragmaAllocSection() {
  assert(Tok.is(tok::annot_pragma_alloc_section));
  SourceLocation AllocSectionLoc = ConsumeToken();
  SmallVector<ExprResult, 4> VarNames;
  ExprResult Section;
  unsigned locId = 0;
  bool IsCorrect = true;

  // Lex the left '(', if any, and just ignore
  if (Tok.is(tok::l_paren)) {
    PP.Lex(Tok);
  }
  else {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_lparen) << "alloc_section";
    locId = Tok.getLocation().getRawEncoding();
  }

  while (Tok.isAnyIdentifier()) {
    CXXScopeSpec SS;
    SourceLocation TemplateKWLoc;
    UnqualifiedId Name;
    // Read var name.
    Token PrevTok = Tok;

    if (getLangOpts().CPlusPlus &&
        ParseOptionalCXXScopeSpecifier(SS, ParsedType(), false)) {
      IsCorrect = false;
      while (!SkipUntil(tok::comma, tok::r_paren, tok::annot_pragma_end,
                        StopBeforeMatch));
      DiscardUntilEndOfDirective();
    } else if (ParseUnqualifiedId(SS, false, false, false, ParsedType(),
                                  TemplateKWLoc, Name)) {
      IsCorrect = false;
      while (!SkipUntil(tok::comma, tok::r_paren, tok::annot_pragma_end,
                        StopBeforeMatch));
    } else if (Tok.isNot(tok::comma) && Tok.isNot(tok::r_paren) &&
               Tok.isNot(tok::annot_pragma_end)) {
      IsCorrect = false;
      Diag(PrevTok.getLocation(), diag::err_expected_ident)
        << SourceRange(PrevTok.getLocation(), PrevTokLocation);
      while (!SkipUntil(tok::comma, tok::r_paren, tok::annot_pragma_end,
                        StopBeforeMatch));
    } else {
      DeclarationNameInfo NameInfo = Actions.GetNameFromUnqualifiedId(Name);
      ExprResult Res = Actions.ActOnCustomIdExpression(getCurScope(), SS,
                                                       NameInfo);
      if (Res.isUsable())
        VarNames.push_back(Res.get());
    }
    // Consume ','.
    if (Tok.is(tok::comma)) {
      ConsumeAnyToken();
    }
  }

  // Lex string literal with section name
  if (Tok.isNot(tok::string_literal)) {
    if (locId != Tok.getLocation().getRawEncoding())
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_string);
    DiscardUntilEndOfDirective();
    return (StmtEmpty());
  }
  else {
    Section = ParseStringLiteralExpression();
    if (!isa<StringLiteral>(Section.get()) || cast<StringLiteral>(Section.get())->getLength() == 0) {
      if (locId != Tok.getLocation().getRawEncoding())
        PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_no_null_string);
      DiscardUntilEndOfDirective();
      return (StmtEmpty());
    }
    StringRef SectName = cast<StringLiteral>(Section.get())->getBytes();
    if (SectName != "short" && SectName != "long") {
      if (locId != Tok.getLocation().getRawEncoding())
        PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_unexpected_section)<<SectName;
      DiscardUntilEndOfDirective();
      return (StmtEmpty());
    }
  }

  // Lex the ')', if any, and just ignore
  if (Tok.is(tok::r_paren)) {
    PP.Lex(Tok);
  }
  else {
    if (locId != Tok.getLocation().getRawEncoding())
      PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_rparen) << "alloc_section";
    locId = Tok.getLocation().getRawEncoding();
  }

  if (Tok.isNot(tok::annot_pragma_end)) {
    if (locId != Tok.getLocation().getRawEncoding())
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_extra_text);
  }
  DiscardUntilEndOfDirective();
  if (!IsCorrect) return StmtError();

  return (Actions.ActOnPragmaOptionsAllocSection(AllocSectionLoc, VarNames, Section));
}

void Parser::HandlePragmaAllocSectionDecl() {
  assert(Tok.is(tok::annot_pragma_alloc_section));
  StmtResult Res = HandlePragmaAllocSection();
  
  if (Res.get() && !(static_cast<PragmaStmt *>(Res.get()))->isNullOp())
    Actions.ActOnPragmaOptionsAllocSection(static_cast<PragmaStmt *>(Res.get()));
}

void PragmaAllocSectionHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation AllocSectionLoc = FirstTok.getLocation();

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_alloc_section);
  Tok.setLocation(AllocSectionLoc);
  Tok.setAnnotationValue(static_cast<void*>(0));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  AllocSectionLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(AllocSectionLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

// #pragma section ("string", attribute1, attribute2, ...)

StmtResult Parser::HandlePragmaSection() {
  assert(Tok.is(tok::annot_pragma_section));
  SourceLocation SectionLoc = ConsumeToken();
  SmallVector<Token, 4> AttrNames;
  ExprResult Section;
  unsigned locId = 0;

  // Lex the left '(', if any, and just ignore
  if (Tok.is(tok::l_paren)) {
    PP.Lex(Tok);
  }
  else {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_lparen) << "section";
    locId = Tok.getLocation().getRawEncoding();
  }

  // Lex string literal with section name
  if (Tok.isNot(tok::string_literal)) {
    if (locId != Tok.getLocation().getRawEncoding())
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_string);
    DiscardUntilEndOfDirective();
    return (StmtEmpty());
  }
  else {
    Section = ParseStringLiteralExpression();
  }

  // Lex the ',', if any, and just ignore
  if (Tok.is(tok::comma)) {
    PP.Lex(Tok);
  }

  while (Tok.isAnyIdentifier() || Tok.is(tok::kw_long) || Tok.is(tok::kw_short)) {
    AttrNames.push_back(Tok);
    PP.Lex(Tok);
    // Lex the ',', if any, and just ignore
    if (Tok.is(tok::comma)) {
      PP.Lex(Tok);
    }
  }

  // Lex the ')', if any, and just ignore
  if (Tok.is(tok::r_paren)) {
    PP.Lex(Tok);
  }
  else {
    if (locId != Tok.getLocation().getRawEncoding())
      PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_rparen) << "section";
    locId = Tok.getLocation().getRawEncoding();
  }

  if (Tok.isNot(tok::annot_pragma_end)) {
    if (locId != Tok.getLocation().getRawEncoding())
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_extra_text);
  }
  DiscardUntilEndOfDirective();

  return (Actions.ActOnPragmaOptionsSection(SectionLoc, Section, AttrNames));
}

void Parser::HandlePragmaSectionDecl() {
  assert(Tok.is(tok::annot_pragma_section));
  StmtResult Res = HandlePragmaSection();
  
  if (Res.get() && !(static_cast<PragmaStmt *>(Res.get()))->isNullOp())
    Actions.ActOnPragmaOptionsSection(static_cast<PragmaStmt *>(Res.get()));
}

void PragmaSectionHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation SectionLoc = FirstTok.getLocation();

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_section);
  Tok.setLocation(SectionLoc);
  Tok.setAnnotationValue(static_cast<void*>(0));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  SectionLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(SectionLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

// #pragma alloc_text ("string", function1, function2, ...)

StmtResult Parser::HandlePragmaAllocText() {
  assert(Tok.is(tok::annot_pragma_alloc_text));
  SourceLocation AllocTextLoc = ConsumeToken();
  SmallVector<ExprResult, 4> FuncNames;
  ExprResult Section;
  unsigned locId = 0;

  // Lex the left '(', if any, and just ignore
  if (Tok.is(tok::l_paren)) {
    PP.Lex(Tok);
  }
  else {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_lparen) << "alloc_text";
    locId = Tok.getLocation().getRawEncoding();
  }

  // Lex string literal with section name
  if (Tok.isNot(tok::string_literal)) {
    if (locId != Tok.getLocation().getRawEncoding())
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_string);
    DiscardUntilEndOfDirective();
    return (StmtEmpty());
  }
  else {
    Section = ParseStringLiteralExpression();
    if (!isa<StringLiteral>(Section.get()) || cast<StringLiteral>(Section.get())->getLength() == 0) {
      if (locId != Tok.getLocation().getRawEncoding())
        PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_no_null_string);
      DiscardUntilEndOfDirective();
      return (StmtEmpty());
    }
  }

  // Lex the ',', if any, and just ignore
  if (Tok.is(tok::comma)) {
    PP.Lex(Tok);
  }

  bool IsCorrect = true;
  while (Tok.isAnyIdentifier()) {
    CXXScopeSpec SS;
    SourceLocation TemplateKWLoc;
    UnqualifiedId Name;
    // Read var name.
    Token PrevTok = Tok;

    if (getLangOpts().CPlusPlus &&
        ParseOptionalCXXScopeSpecifier(SS, ParsedType(), false)) {
      IsCorrect = false;
      while (!SkipUntil(tok::comma, tok::r_paren, tok::annot_pragma_end,
                        StopBeforeMatch));
      DiscardUntilEndOfDirective();
    } else if (ParseUnqualifiedId(SS, false, false, false, ParsedType(),
                                  TemplateKWLoc, Name)) {
      IsCorrect = false;
      while (!SkipUntil(tok::comma, tok::r_paren, tok::annot_pragma_end,
                        StopBeforeMatch));
    } else if (Tok.isNot(tok::comma) && Tok.isNot(tok::r_paren) &&
               Tok.isNot(tok::annot_pragma_end)) {
      IsCorrect = false;
      Diag(PrevTok.getLocation(), diag::err_expected_ident)
        << SourceRange(PrevTok.getLocation(), PrevTokLocation);
      while (!SkipUntil(tok::comma, tok::r_paren, tok::annot_pragma_end,
                        StopBeforeMatch));
    } else {
      DeclarationNameInfo NameInfo = Actions.GetNameFromUnqualifiedId(Name);
      ExprResult Res = Actions.ActOnCustomIdExpression(getCurScope(), SS,
                                                       NameInfo);
      if (Res.isUsable())
        FuncNames.push_back(Res.get());
    }
    // Consume ','.
    if (Tok.is(tok::comma)) {
      ConsumeAnyToken();
    }
  }
  if (FuncNames.empty()) {
    if (locId != Tok.getLocation().getRawEncoding())
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_func_expect);
    DiscardUntilEndOfDirective();
    return (StmtEmpty());
  }

  // Lex the ')', if any, and just ignore
  if (Tok.is(tok::r_paren)) {
    PP.Lex(Tok);
  }
  else {
    if (locId != Tok.getLocation().getRawEncoding())
      PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_rparen) << "alloc_text";
    locId = Tok.getLocation().getRawEncoding();
  }

  if (Tok.isNot(tok::annot_pragma_end)) {
    if (locId != Tok.getLocation().getRawEncoding())
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_extra_text);
  }
  DiscardUntilEndOfDirective();
  if (!IsCorrect) return StmtError();

  return (Actions.ActOnPragmaOptionsAllocText(AllocTextLoc, Section, FuncNames));
}

void Parser::HandlePragmaAllocTextDecl() {
  assert(Tok.is(tok::annot_pragma_alloc_text));
  StmtResult Res = HandlePragmaAllocText();
  
  if (Res.get() && !(static_cast<PragmaStmt *>(Res.get()))->isNullOp())
    Actions.ActOnPragmaOptionsAllocText(static_cast<PragmaStmt *>(Res.get()));
}

void PragmaAllocTextHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation AllocTextLoc = FirstTok.getLocation();

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_alloc_text);
  Tok.setLocation(AllocTextLoc);
  Tok.setAnnotationValue(static_cast<void*>(0));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  AllocTextLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(AllocTextLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

// #pragma auto_inline ([on|off])

StmtResult Parser::HandlePragmaAutoInline() {
  assert(Tok.is(tok::annot_pragma_auto_inline));
  SourceLocation AutoInlineLoc = ConsumeToken();
  Sema::IntelPragmaAutoInlineOption Opt = Sema::IntelPragmaAutoInlineOptionOff;

  // Lex the left '(', if any, and just ignore
  if (Tok.is(tok::l_paren)) {
    PP.Lex(Tok);
  }

  if (Tok.isNot(tok::annot_pragma_end)) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    if (!II || !(II->isStr("on") || II->isStr("off"))) {
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_on_off_expected);
      DiscardUntilEndOfDirective();
      return (StmtEmpty());
    } else {
      Opt = (II->isStr("on")) ? Sema::IntelPragmaAutoInlineOptionOn : Sema::IntelPragmaAutoInlineOptionOff;
      PP.Lex(Tok);
    }
  }
  // Lex the ')', if any, and just ignore
  if (Tok.is(tok::r_paren)) {
    PP.Lex(Tok);
  }
  if (Tok.isNot(tok::annot_pragma_end)) {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_extra_text);
  }
  DiscardUntilEndOfDirective();

  return (Actions.ActOnPragmaOptionsAutoInline(AutoInlineLoc, Opt));
}

void Parser::HandlePragmaAutoInlineDecl() {
  assert(Tok.is(tok::annot_pragma_auto_inline));
  StmtResult Res = HandlePragmaAutoInline();
  
  if (Res.get() && !(static_cast<PragmaStmt *>(Res.get()))->isNullOp())
    Actions.ActOnPragmaOptionsAutoInline(static_cast<PragmaStmt *>(Res.get()));
}

void PragmaAutoInlineHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation AutoInlineLoc = FirstTok.getLocation();

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_auto_inline);
  Tok.setLocation(AutoInlineLoc);
  Tok.setAnnotationValue(static_cast<void*>(0));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  AutoInlineLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(AutoInlineLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

// Pragmas {bss_seg|code_seg|const_seg|data_seg} ([[{push|pop}, ] [identifier ,]]["segment-name"[, "segment-class"])

StmtResult Parser::HandlePragmaSeg() {
  assert(Tok.is(tok::annot_pragma_seg));
  const Sema::IntelPragmaSegKind *KindPtr = 
    static_cast<Sema::IntelPragmaSegKind *>(Tok.getAnnotationValue());
  const Sema::IntelPragmaSegKind Kind = *KindPtr;
  SourceLocation SegLoc = ConsumeToken();
  Sema::IntelPragmaSegOption Opt = Sema::IntelPragmaSegOptionSet;
  std::string Identifier;
  bool IdentifierSet = false;
  std::string SegName;
  bool SegNameSet = false;
  std::string ClassName;
  bool ClassNameSet = false;

  delete KindPtr;

  // Lex the left '(', if any, and just ignore
  if (Tok.is(tok::l_paren)) {
    PP.Lex(Tok);
  }

  if (Tok.isNot(tok::annot_pragma_end)) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    if (II) {
      if (II->isStr("push")) {
        Opt = Sema::IntelPragmaSegOptionPush;
      }
      else if (II->isStr("pop")) {
        Opt = Sema::IntelPragmaSegOptionPop;
      }
      PP.Lex(Tok);
      // Lex the ',', if any, and just ignore
      if (Tok.is(tok::comma)) {
        PP.Lex(Tok);
      }
      if (Tok.isAnyIdentifier()) {
        const IdentifierInfo *II = Tok.getIdentifierInfo();
        if (II) {
          Identifier = II->getName().str();
          IdentifierSet = true;
        }
        PP.Lex(Tok);
        // Lex the ',', if any, and just ignore
        if (Tok.is(tok::comma)) {
          PP.Lex(Tok);
        }
      }
    }
  }
  // Section name
  if (Tok.is(tok::string_literal)) {
    SegName = StringRef(Tok.getLiteralData(), Tok.getLength()).str();
    SegNameSet = true;
    if (SegName.empty() || SegName == "\"\"") {
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_no_null_string);
      DiscardUntilEndOfDirective();
      return (StmtEmpty());
    }
    PP.Lex(Tok);
    // Lex the ',', if any, and just ignore
    if (Tok.is(tok::comma)) {
      PP.Lex(Tok);
      if (Tok.isNot(tok::string_literal)) {
        PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_string);
        DiscardUntilEndOfDirective();
        return (StmtEmpty());
      }
      else {
        ClassName = StringRef(Tok.getLiteralData(), Tok.getLength()).str();
        ClassNameSet = true;
        if (ClassName.empty() || ClassName == "\"\"") {
          PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_no_null_string);
          DiscardUntilEndOfDirective();
          return (StmtEmpty());
        }
        PP.Lex(Tok);
      }
    }
  }

  // Lex the ')', if any, and just ignore
  if (Tok.is(tok::r_paren)) {
    PP.Lex(Tok);
  }

  if (Tok.isNot(tok::annot_pragma_end)) {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_extra_text);
  }
  DiscardUntilEndOfDirective();

  return (Actions.ActOnPragmaOptionsSeg(SegLoc, Kind, Opt, IdentifierSet, Identifier, SegNameSet, SegName, ClassNameSet, ClassName));
}

void Parser::HandlePragmaSegDecl() {
  assert(Tok.is(tok::annot_pragma_seg));
  StmtResult Res = HandlePragmaSeg();

  if (Res.get() && !(static_cast<PragmaStmt *>(Res.get()))->isNullOp())
    Actions.ActOnPragmaOptionsSeg(static_cast<PragmaStmt *>(Res.get()));
}

void PragmaBssSegHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation SegLoc = FirstTok.getLocation();
  Sema::IntelPragmaSegKind *KindPtr = new Sema::IntelPragmaSegKind;

  *KindPtr = Sema::IntelPragmaBssSeg;

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_seg);
  Tok.setLocation(SegLoc);
  Tok.setAnnotationValue(static_cast<void*>(KindPtr));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  SegLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(SegLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

void PragmaCodeSegHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation SegLoc = FirstTok.getLocation();
  Sema::IntelPragmaSegKind *KindPtr = new Sema::IntelPragmaSegKind;

  *KindPtr = Sema::IntelPragmaCodeSeg;

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_seg);
  Tok.setLocation(SegLoc);
  Tok.setAnnotationValue(static_cast<void*>(KindPtr));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  SegLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(SegLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

void PragmaConstSegHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation SegLoc = FirstTok.getLocation();
  Sema::IntelPragmaSegKind *KindPtr = new Sema::IntelPragmaSegKind;

  *KindPtr = Sema::IntelPragmaConstSeg;

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_seg);
  Tok.setLocation(SegLoc);
  Tok.setAnnotationValue(static_cast<void*>(KindPtr));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  SegLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(SegLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

void PragmaDataSegHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation SegLoc = FirstTok.getLocation();
  Sema::IntelPragmaSegKind *KindPtr = new Sema::IntelPragmaSegKind;

  *KindPtr = Sema::IntelPragmaDataSeg;

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_seg);
  Tok.setLocation(SegLoc);
  Tok.setAnnotationValue(static_cast<void*>(KindPtr));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  SegLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(SegLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

// #pragma check_stack([on|off|+|-])

StmtResult Parser::HandlePragmaCheckStack() {
  assert(Tok.is(tok::annot_pragma_check_stack));
  SourceLocation CheckStackLoc = ConsumeToken();
  Sema::IntelPragmaCheckStackOption Opt = Sema::IntelPragmaCheckStackOptionOff;

  // Lex the left '(', if any, and just ignore
  if (Tok.is(tok::l_paren)) {
    PP.Lex(Tok);
  }

  if (Tok.isAnyIdentifier()) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    if (!II || !(II->isStr("on") || II->isStr("off"))) {
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_on_off_expected);
      DiscardUntilEndOfDirective();
      return (StmtEmpty());
    } else {
      Opt = (II->isStr("on")) ? Sema::IntelPragmaCheckStackOptionOn : Sema::IntelPragmaCheckStackOptionOff;
      PP.Lex(Tok);
    }
  }
  else if (Tok.is(tok::plus) || Tok.is(tok::minus)) {
    Opt = Tok.is(tok::plus) ? Sema::IntelPragmaCheckStackOptionOn : Sema::IntelPragmaCheckStackOptionOff;
    PP.Lex(Tok);
  }
  else {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_on_off_expected);
    DiscardUntilEndOfDirective();
    return (StmtEmpty());
  }
  // Lex the ')', if any, and just ignore
  if (Tok.is(tok::r_paren)) {
    PP.Lex(Tok);
  }
  if (Tok.isNot(tok::annot_pragma_end)) {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_extra_text);
  }
  DiscardUntilEndOfDirective();

  return (Actions.ActOnPragmaOptionsCheckStack(CheckStackLoc, Opt));
}

void Parser::HandlePragmaCheckStackDecl() {
  assert(Tok.is(tok::annot_pragma_check_stack));
  StmtResult Res = HandlePragmaCheckStack();
  
  if (Res.get() && !(static_cast<PragmaStmt *>(Res.get()))->isNullOp())
    Actions.ActOnPragmaOptionsCheckStack(static_cast<PragmaStmt *>(Res.get()));
}

void PragmaCheckStackHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation CheckStackLoc = FirstTok.getLocation();

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_check_stack);
  Tok.setLocation(CheckStackLoc);
  Tok.setAnnotationValue(static_cast<void*>(0));
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  CheckStackLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(CheckStackLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

// #pragma component
void PragmaComponentHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  // ignore everything till the end of line
  PP.DiscardUntilEndOfDirective();
}

// #pragma conform
struct ForScopeInfo {
  std::string Name;
  bool forScope;
  ForScopeInfo(const std::string &N, bool fS) : Name(N), forScope(fS) {}
};

static SmallVector<ForScopeInfo, 4> ForScopeVector;

void PragmaConformHandler::Push_forScope(Preprocessor &PP, Token &Tok) {
  bool Operation = (PP.getLangOpts().Zc_forScope != 0);
  std::string Name;

  // Lex the ',', if any
  if (Tok.is(tok::comma)) {
    PP.Lex(Tok);
    const IdentifierInfo *II;
    if (Tok.isAnyIdentifier() && (II = Tok.getIdentifierInfo()) && !(II->isStr("on") || II->isStr("off"))) {
      Name = II->getName().str();
      PP.Lex(Tok);
      if (Tok.is(tok::r_paren)) {
        ForScopeVector.push_back(ForScopeInfo(Name, Operation));
        if (Tok.isNot(tok::eod)) {
          PP.DiscardUntilEndOfDirective();
        }
        return;
      }
      else if (Tok.is(tok::comma)) {
        PP.Lex(Tok);
      }
      else {
        PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"',' or ')'"<<0;
        if (Tok.isNot(tok::eod)) {
          PP.DiscardUntilEndOfDirective();
        }
        return;
      }
    }
    if (Tok.isAnyIdentifier() && (II = Tok.getIdentifierInfo()) && (II->isStr("on") || II->isStr("off"))) {
      Operation = II->isStr("on");
      PP.Lex(Tok);
      if (Tok.is(tok::r_paren)) {
        ForScopeVector.push_back(ForScopeInfo(Name, Operation));
        PP.getLangOpts().Zc_forScope = Operation ? 1 : 0;
        if (Tok.isNot(tok::eod)) {
          PP.DiscardUntilEndOfDirective();
        }
        return;
      }
      else {
        PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_rparen)<<"conform";
        if (Tok.isNot(tok::eod)) {
          PP.DiscardUntilEndOfDirective();
        }
        return;
      }
    }
    else {
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"'on', 'off' or another identifier"<<0;
      if (Tok.isNot(tok::eod)) {
        PP.DiscardUntilEndOfDirective();
      }
      return;
    }
  }
  else if (Tok.is(tok::r_paren)) {
    ForScopeVector.push_back(ForScopeInfo(Name, Operation));
    if (Tok.isNot(tok::eod)) {
      PP.DiscardUntilEndOfDirective();
    }
  }
  else {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"',' or ')'"<<0;
    if (Tok.isNot(tok::eod)) {
      PP.DiscardUntilEndOfDirective();
    }
  }
}

void PragmaConformHandler::Pop_forScope(Preprocessor &PP, Token &Tok) {
  // Lex the ',', if any
  if (Tok.is(tok::comma)) {
    PP.Lex(Tok);
    const IdentifierInfo *II;
    if (Tok.isAnyIdentifier() && (II = Tok.getIdentifierInfo())) {
      PP.Lex(Tok);
      if (Tok.is(tok::r_paren)) {
        std::string Name = II->getName().str();
        for (unsigned i = ForScopeVector.size(); i > 0; --i) {
          if (ForScopeVector[i - 1].Name == Name) {
            for (unsigned j = ForScopeVector.size(); j >= i; --j) {
              ForScopeVector.pop_back();
            }
            if (!ForScopeVector.empty()) {
              PP.getLangOpts().Zc_forScope = ForScopeVector.back().forScope ? 1 : 0;
            }
            break;
          }
        }
      }
      else {
        PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_rparen)<<"conform";
        if (Tok.isNot(tok::eod)) {
          PP.DiscardUntilEndOfDirective();
        }
      }
    }
    else {
      PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_identifier)<<"conform";
      if (Tok.isNot(tok::eod)) {
        PP.DiscardUntilEndOfDirective();
      }
    }
  }
  else if (Tok.is(tok::r_paren)) {
    if (!ForScopeVector.empty()) {
      PP.getLangOpts().Zc_forScope = ForScopeVector.back().forScope ? 1 : 0;
      ForScopeVector.pop_back();
    }
  }
  else {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"',' or ')'"<<0;
    if (Tok.isNot(tok::eod)) {
      PP.DiscardUntilEndOfDirective();
    }
  }
}

void PragmaConformHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SourceLocation ConformLoc = FirstTok.getLocation();

  PP.Lex(Tok);
  // Lex the '(', if any
  if (Tok.isNot(tok::l_paren)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_lparen)<<"conform";
    if (Tok.isNot(tok::eod)) {
      PP.DiscardUntilEndOfDirective();
    }
    return;
  }
  else {
    PP.Lex(Tok);
  }

  if (Tok.isAnyIdentifier()) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    if (!II || !II->isStr("forScope")) {
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"'forScope'"<<0;
      if (Tok.isNot(tok::eod)) {
        PP.DiscardUntilEndOfDirective();
      }
      return;
    } else {
      PP.Lex(Tok);
    }
  }
  else {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"'forScope'"<<0;
    if (Tok.isNot(tok::eod)) {
      PP.DiscardUntilEndOfDirective();
    }
    return;
  }

  // Lex the ',', if any
  if (Tok.is(tok::comma)) {
    PP.Lex(Tok);
  }
  else {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"','"<<0;
    if (Tok.isNot(tok::eod)) {
      PP.DiscardUntilEndOfDirective();
    }
    return;
  }

  if (Tok.isAnyIdentifier()) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    if (!II || 
      !(II->isStr("show") || II->isStr("push") || II->isStr("pop") || II->isStr("on") || II->isStr("off"))) {
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"'show', 'push', 'pop', 'on' or 'off'"<<0;
      if (Tok.isNot(tok::eod)) {
        PP.DiscardUntilEndOfDirective();
      }
      return;
    } else {
      PP.Lex(Tok);
      if (II->isStr("push")) {
        Push_forScope(PP, Tok);
        return;
      }
      else if (II->isStr("pop")) {
        Pop_forScope(PP, Tok);
        return;
      }
      else if (II->isStr("on")) {
        if (Tok.is(tok::r_paren))
          PP.getLangOpts().Zc_forScope = 1;
      }
      else if (II->isStr("off")) {
        if (Tok.is(tok::r_paren))
          PP.getLangOpts().Zc_forScope = 0;
      }
      else {
        if (Tok.is(tok::r_paren))
          PP.Diag(ConformLoc, diag::x_warn_intel_pragma_conform_value)<<PP.getLangOpts().Zc_forScope;
      }
    }
  }
  else {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"'show', 'push', 'pop', 'on' or 'off'"<<0;
    if (Tok.isNot(tok::eod)) {
      PP.DiscardUntilEndOfDirective();
    }
    return;
  }

  // Lex the ')', if any
  if (Tok.isNot(tok::r_paren)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_rparen)<<"conform";
  }
  else {
    PP.Lex(Tok);
  }

  // ignore everything till the end of line
  if (Tok.isNot(tok::eod)) {
    PP.DiscardUntilEndOfDirective();
  }
}

// #pragma deprecated
void PragmaDeprecatedHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  // ignore everything till the end of line
  PP.DiscardUntilEndOfDirective();
}

// #pragma fp_contract
StmtResult Parser::HandlePragmaCommonOnOff(Sema::IntelPragmaCommonOnOff Kind, bool isDefaultAllowed) {
  assert(Tok.is(tok::annot_pragma_intel_fp_contract) || Tok.is(tok::annot_pragma_fenv_access));
  SourceLocation Loc = ConsumeToken();
  Sema::IntelCommonDefaultOnOff DOO;
  std::string Pragma;
  std::string SpecPragma;

  switch (Kind) {
    case (Sema::IntelPragmaFPContract):
      Pragma = "fp_contract";
      SpecPragma = "FP_CONTRACT";
      break;
    case (Sema::IntelPragmaFEnvAccess):
      Pragma = "fenv_access";
      SpecPragma = "FENV_ACCESS";
      break;
  }

  // Lex the '(', if any
  if (Tok.isNot(tok::l_paren)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_lparen)<<Pragma;
    DiscardUntilEndOfDirective();
    return (StmtEmpty());
  }
  else {
    PP.Lex(Tok);
  }

  if (Tok.isAnyIdentifier()) {
    if (isDefaultAllowed && Tok.is(tok::kw_default)) {
      DOO = Sema::IntelCommonDefault;
    }
    else {
      const IdentifierInfo *II = Tok.getIdentifierInfo();
      if (!II || !(II->isStr("on") || II->isStr("off"))) {
        PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<(isDefaultAllowed ? "'default', 'on' or 'off'" : "'on' or 'off'")<<0;
        DiscardUntilEndOfDirective();
        return (StmtEmpty());
      } else {
        PP.Lex(Tok);
        //OOS = II->isStr("on") ? tok::OOS_ON : tok::OOS_OFF;
        DOO = II->isStr("on") ? Sema::IntelCommonOn : Sema::IntelCommonOff;
      }
    }
  }
  else {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<(isDefaultAllowed ? "'default', 'on' or 'off'" : "'on' or 'off'")<<0;
    DiscardUntilEndOfDirective();
    return (StmtEmpty());
  }

  // Lex the ')', if any
  if (Tok.isNot(tok::r_paren)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_rparen)<<Pragma;
  }
  else {
    PP.Lex(Tok);
  }

  if (Tok.isNot(tok::annot_pragma_end)) {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_extra_text);
  }

  // ignore everything till the end of line
  DiscardUntilEndOfDirective();

  //Actions.ActOnPragmaFPContract(OOS);
  return (Actions.ActOnPragmaCommonOnOff(Loc, Pragma.data(), SpecPragma.data(), DOO, Kind, FPState));
}

void Parser::HandlePragmaCommonOnOffDecl(Sema::IntelPragmaCommonOnOff Kind, bool isDefaultAllowed) {
  assert(Tok.is(tok::annot_pragma_intel_fp_contract) || Tok.is(tok::annot_pragma_fenv_access));
  StmtResult Res = HandlePragmaCommonOnOff(Kind, isDefaultAllowed);
  std::string SpecPragma;
  switch (Kind) {
    case (Sema::IntelPragmaFPContract):
      SpecPragma = "FP_CONTRACT";
      break;
    case (Sema::IntelPragmaFEnvAccess):
      SpecPragma = "FENV_ACCESS";
      break;
  }

  if (Res.get() && !(static_cast<PragmaStmt *>(Res.get()))->isNullOp())
    Actions.ActOnPragmaCommonOnOff(static_cast<PragmaStmt *>(Res.get()), SpecPragma.data());
}

void PragmaIntelFPContractHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation Loc = FirstTok.getLocation();

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_intel_fp_contract);
  Tok.setLocation(Loc);
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  Loc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(Loc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

void PragmaIntelFenvAccessHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation Loc = FirstTok.getLocation();

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_fenv_access);
  Tok.setLocation(Loc);
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  Loc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(Loc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

// Pragmas init_seg (compiler|lib|user|"segment-class")

StmtResult Parser::HandlePragmaInitSeg() {
  assert(Tok.is(tok::annot_pragma_init_seg));
  SourceLocation SegLoc = ConsumeToken();
  std::string Section;

  // Lex the left '(', if any, and just ignore
  if (Tok.is(tok::l_paren)) {
    PP.Lex(Tok);
  }

  if (Tok.isAnyIdentifier() && Tok.getIdentifierInfo()) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    PP.Lex(Tok);
    if (!(II->isStr("compiler") || II->isStr("lib") || II->isStr("user"))) {
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"'compiler', 'lib', 'user' or section name"<<0;
      DiscardUntilEndOfDirective();
      return StmtEmpty();
    }
    Section = II->getName().str();
  }
  else if (Tok.is(tok::string_literal)) {
    Section = StringRef(Tok.getLiteralData(), Tok.getLength()).str();
    if (Section.empty() || Section == "\"\"") {
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_no_null_string);
      DiscardUntilEndOfDirective();
      return (StmtEmpty());
    }
    PP.Lex(Tok);
  }
  else {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"'compiler', 'lib', 'user' or section name"<<0;
    DiscardUntilEndOfDirective();
    return (StmtEmpty());
  }

  // Lex the left ')', if any, and just ignore
  if (Tok.is(tok::r_paren)) {
    PP.Lex(Tok);
  }

  if (Tok.isNot(tok::annot_pragma_end)) {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_extra_text);
  }
  DiscardUntilEndOfDirective();

  return (Actions.ActOnPragmaOptionsInitSeg(SegLoc, Section));
}

void Parser::HandlePragmaInitSegDecl() {
  assert(Tok.is(tok::annot_pragma_init_seg));
  StmtResult Res = HandlePragmaInitSeg();

  if (Res.get() && !(static_cast<PragmaStmt *>(Res.get()))->isNullOp())
    Actions.ActOnPragmaOptionsInitSeg(static_cast<PragmaStmt *>(Res.get()));
}

void PragmaInitSegHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation SegLoc = FirstTok.getLocation();

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_init_seg);
  Tok.setLocation(SegLoc);
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  SegLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(SegLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

// #pragma float_control (push | pop)
// #pragma float_control (precise | source | double | extended | except, on | off [, push])
StmtResult Parser::HandlePragmaFloatControl() {
  assert(Tok.is(tok::annot_pragma_float_control));
  SourceLocation FloatControlLoc = ConsumeToken();
  Sema::IntelPragmaFloatControlOnOff OOS;
  Sema::IntelPragmaFloatControlOption Kind;
  bool NeedToPush = false;

  // Lex the '(', if any
  if (Tok.isNot(tok::l_paren)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_lparen)<<"float_control";
    DiscardUntilEndOfDirective();
    return (StmtEmpty());
  }
  else {
    PP.Lex(Tok);
  }

  // push, pop, precise, source, double, extended, except
  if (!Tok.isAnnotation()) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    if (!II || !(II->isStr("push") || II->isStr("pop") || II->isStr("precise") || II->isStr("source")
      || II->isStr("double") || II->isStr("extended") || II->isStr("except"))) {
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<
        "'push', 'pop', 'precise', 'source', 'double', 'extended' or 'except'"<<0;
      DiscardUntilEndOfDirective();
      return (StmtEmpty());
    } else {
      PP.Lex(Tok);
      if (II->isStr("push")) {
        // Lex the ')', if any
        if (Tok.isNot(tok::r_paren)) {
          PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_rparen)<<"float_control";
          DiscardUntilEndOfDirective();
          return (StmtEmpty());
        }
        // ignore everything till the end of line
        DiscardUntilEndOfDirective();
        // push
        FCVector.push_back(FPState);
        return (StmtEmpty());
      }
      else if (II->isStr("pop")) {
        // Lex the ')', if any
        if (Tok.isNot(tok::r_paren)) {
          PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_rparen)<<"float_control";
          DiscardUntilEndOfDirective();
          return (StmtEmpty());
        }
        // ignore everything till the end of line
        DiscardUntilEndOfDirective();
        // pop
        if (!FCVector.empty()) {
          if (FCVector.size() > 1)
            FCVector.pop_back();
          if (!FCVector.empty()) {
            FPState = FCVector.back();
            return (Actions.ActOnPragmaOptionsFloatControl(FloatControlLoc, FPState));
          }
        }
        return (StmtEmpty());
      }
      else if (II->isStr("precise")) {
        // precise
        Kind = Sema::IntelPragmaFloatControlPrecise;
      }
      else if (II->isStr("source")) {
        // source
        Kind = Sema::IntelPragmaFloatControlSource;
      }
      else if (II->isStr("double")) {
        // double
        Kind = Sema::IntelPragmaFloatControlDouble;
      }
      else if (II->isStr("extended")) {
        // extended
        Kind = Sema::IntelPragmaFloatControlExtended;
      }
      else {
        // except
        Kind = Sema::IntelPragmaFloatControlExcept;
      }
    }
  }
  else {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<
      "'push', 'pop', 'precise', 'source', 'double', 'extended' or 'except'"<<0;
    DiscardUntilEndOfDirective();
    return (StmtEmpty());
  }

  // Lex the ',', if any, and just ignore
  if (Tok.is(tok::comma)) {
    PP.Lex(Tok);
  }
  else {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"','"<<0;
    DiscardUntilEndOfDirective();
    return (StmtEmpty());
  }

  // on or off
  if (Tok.isAnyIdentifier()) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    if (!II || !(II->isStr("on") || II->isStr("off"))) {
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"'on' or 'off'"<<0;
      DiscardUntilEndOfDirective();
      return (StmtEmpty());
    } else {
      PP.Lex(Tok);
      OOS = II->isStr("on") ? Sema::IntelPragmaFloatControlOn : Sema::IntelPragmaFloatControlOff;
    }
  }
  else {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"'on' or 'off'"<<0;
    DiscardUntilEndOfDirective();
    return (StmtEmpty());
  }

  // Lex the ',', if any, and just ignore
  if (Tok.is(tok::comma)) {
    PP.Lex(Tok);
    // push
    if (Tok.isAnyIdentifier()) {
      const IdentifierInfo *II = Tok.getIdentifierInfo();
      if (!II || !II->isStr("push")) {
        PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"'push'"<<0;
        DiscardUntilEndOfDirective();
        return (StmtEmpty());
      } else {
        PP.Lex(Tok);
        // push
        NeedToPush = true;
      }
    }
    else {
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"'push'"<<0;
      DiscardUntilEndOfDirective();
      return (StmtEmpty());
    }
  }

  // Lex the ')', if any
  if (Tok.isNot(tok::r_paren)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_rparen)<<"float_control";
    DiscardUntilEndOfDirective();
    return (StmtEmpty());
  }
  else {
    PP.Lex(Tok);
  }

  if (Tok.isNot(tok::annot_pragma_end)) {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_extra_text);
  }
  DiscardUntilEndOfDirective();

  StmtResult Res = Actions.ActOnPragmaOptionsFloatControl(FloatControlLoc, FPState, Kind, OOS);

  if (Res.get() && !(static_cast<PragmaStmt *>(Res.get()))->isNullOp()) {
    if (NeedToPush) {
      FCVector.push_back(FPState);
    }
    else if (!FCVector.empty()) {
      FCVector.back() = FPState;
    }
  }

  return (Res);
}

void Parser::HandlePragmaFloatControlDecl() {
  assert(Tok.is(tok::annot_pragma_float_control));
  StmtResult Res = HandlePragmaFloatControl();

  if (Res.get())
    Actions.ActOnPragmaOptionsFloatControl(Res.get());
}

void PragmaFloatControlHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  SmallVector<Token, 4> Tokens;
  SourceLocation FloatControlLoc = FirstTok.getLocation();

  Tok.startToken();
  Tok.setKind(tok::annot_pragma_float_control);
  Tok.setLocation(FloatControlLoc);
  Tokens.push_back(Tok);

  PP.Lex(Tok);
  while (Tok.isNot(tok::eod)) {
    Tokens.push_back(Tok);
    PP.Lex(Tok);
  }
  FloatControlLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_end);
  Tok.setLocation(FloatControlLoc);
  Tokens.push_back(Tok);

  EnterTokenS(PP, Tokens, /*DisableMacroExpansion=*/false);
}

// #pragma region
static llvm::SmallVector<Token, 4> Regions;

void PragmaRegionHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;

  PP.Lex(Tok);
  if (Tok.isNot(tok::eod)) {
    PP.DiscardUntilEndOfDirective();
  }
  Regions.push_back(FirstTok);
}

void PragmaRegionHandler::CheckOpenedRegions(Preprocessor &PP) {
  if(!Regions.empty()) {
    while (!Regions.empty()) {
      PP.Diag(Regions.back().getLocation(), diag::x_warn_intel_pragma_missing_endregion);
      Regions.pop_back();
    }
  }
}

// #pragma endregion
void PragmaEndRegionHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;

  PP.Lex(Tok);
  if (Tok.isNot(tok::eod)) {
    PP.DiscardUntilEndOfDirective();
  }
  if (Regions.empty()) {
    PP.Diag(FirstTok.getLocation(), diag::x_warn_intel_pragma_missing_region);
  }
  else {
    Regions.pop_back();
  }
}

// #pragma start_map_region
void PragmaStartMapRegionHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  Token StrLit;

  PP.Lex(Tok);
  // Lex the '(', if any
  if (Tok.isNot(tok::l_paren)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_lparen)<<"start_map_region";
    if (Tok.isNot(tok::eod)) {
      PP.DiscardUntilEndOfDirective();
    }
    return;
  }
  else {
    PP.Lex(Tok);
  }

  if (Tok.isNot(tok::string_literal)) {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_string);
    if (Tok.isNot(tok::eod)) {
      PP.DiscardUntilEndOfDirective();
    }
    return;
  }
  else {
    StrLit = Tok;
    PP.Lex(Tok);
  }

  // Lex the ')', if any
  if (Tok.isNot(tok::r_paren)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_rparen)<<"start_map_region";
    if (Tok.isNot(tok::eod)) {
      PP.DiscardUntilEndOfDirective();
    }
    return;
  }
  else {
    PP.Lex(Tok);
  }

  if (RegionStarted) {
    if (Tok.isNot(tok::eod)) {
     PP.DiscardUntilEndOfDirective();
    }
    PP.Diag(FirstTok.getLocation(), diag::x_warn_intel_pragma_missing_stop_map_region);
    return;
  }
  RegionStarted = true;
  // Jus leave for now
  PP.ParseStartMapRegion(FirstTok.getLocation(), StrLit);
}

// #pragma stop_map_region
void PragmaStopMapRegionHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;

  PP.Lex(Tok);
  if (Tok.isNot(tok::eod)) {
    PP.DiscardUntilEndOfDirective();
  }
  if (!RegionStarted) {
    PP.Diag(FirstTok.getLocation(), diag::x_warn_intel_pragma_missing_start_map_region);
  }
  RegionStarted = false;
}

// #pragma vtordisp
void PragmaVtorDispHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &FirstTok) {
  Token Tok;
  tok::OnOffSwitch OOS;

  PP.Lex(Tok);
  // Lex the '(', if any
  if (Tok.isNot(tok::l_paren)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_lparen)<<"vtordisp";
    if (Tok.isNot(tok::eod)) {
      PP.DiscardUntilEndOfDirective();
    }
    return;
  }
  else {
    PP.Lex(Tok);
  }

  if (Tok.isAnyIdentifier()) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    if (!II || !(II->isStr("on") || II->isStr("off"))) {
      PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"'on' or 'off'"<<0;
      if (Tok.isNot(tok::eod)) {
        PP.DiscardUntilEndOfDirective();
      }
      return;
    } else {
      PP.Lex(Tok);
      OOS = II->isStr("on") ? tok::OOS_ON : tok::OOS_OFF;
    }
  }
  else {
    PP.Diag(Tok.getLocation(), diag::x_warn_intel_pragma_expected_common)<<"'on' or 'off'"<<0;
    if (Tok.isNot(tok::eod)) {
      PP.DiscardUntilEndOfDirective();
    }
    return;
  }

  // Lex the ')', if any
  if (Tok.isNot(tok::r_paren)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_rparen)<<"vtordisp";
  }
  else {
    PP.Lex(Tok);
  }

  // ignore everything till the end of line
  if (Tok.isNot(tok::eod)) {
    PP.DiscardUntilEndOfDirective();
  }

  PP.getLangOpts().vd = (OOS == tok::OOS_ON) ? 1 : 0;
}
#endif  // INTEL_SPECIFIC_IL0_BACKEND

void Parser::initializeIntelPragmaHandlers() {
#if INTEL_SPECIFIC_CILKPLUS
  if (getLangOpts().CilkPlus) {
    CilkGrainsizeHandler.reset(new PragmaCilkGrainsizeHandler());
    PP.AddPragmaHandler(CilkGrainsizeHandler.get());
    SIMDHandler.reset(new PragmaSIMDHandler());
    PP.AddPragmaHandler(SIMDHandler.get());
  }
#endif // INTEL_SPECIFIC_CILKPLUS

#ifdef INTEL_SPECIFIC_IL0_BACKEND
  if (getLangOpts().IntelCompat) {
    // #pragma ivdep
    IvdepHandler.reset(new PragmaIvdepHandler());
    PP.AddPragmaHandler(IvdepHandler.get());
    // #pragma novector
    NoVectorHandler.reset(new PragmaNoVectorHandler());
    PP.AddPragmaHandler(NoVectorHandler.get());
    // #pragma vector
    VectorHandler.reset(new PragmaVectorHandler());
    PP.AddPragmaHandler(VectorHandler.get());
    // #pragma distribute_point
    DistributeHandler.reset(new PragmaDistributeHandler());
    PP.AddPragmaHandler(DistributeHandler.get());
    DistributeHandler1.reset(new PragmaDistributeHandler1());
    PP.AddPragmaHandler(DistributeHandler1.get());
    // #pragma inline
    InlineHandler.reset(new PragmaInlineHandler());
    PP.AddPragmaHandler(InlineHandler.get());
    // #pragma noinline
    NoInlineHandler.reset(new PragmaNoInlineHandler());
    PP.AddPragmaHandler(NoInlineHandler.get());
    // #pragma forceinline
    ForceInlineHandler.reset(new PragmaForceInlineHandler());
    PP.AddPragmaHandler(ForceInlineHandler.get());
    // #pragma loop_count
    LoopCountHandler.reset(new PragmaLoopCountHandler());
    PP.AddPragmaHandler(LoopCountHandler.get());
    LoopCountHandler1.reset(new PragmaLoopCountHandler1());
    PP.AddPragmaHandler(LoopCountHandler1.get());
    // #pragma optimize
    IntelOptimizeHandler.reset(new PragmaIntelOptimizeHandler());
    PP.AddPragmaHandler(IntelOptimizeHandler.get());
    // #pragma optimization_level
    OptimizationLevelHandler.reset(new PragmaOptimizationLevelHandler(true));
    PP.AddPragmaHandler("intel", OptimizationLevelHandler.get());
    GCCOptimizationLevelHandler.reset(new PragmaOptimizationLevelHandler(false));
    PP.AddPragmaHandler("GCC", GCCOptimizationLevelHandler.get());
    PP.AddPragmaHandler((getLangOpts().PragmaOptimizationLevelIntel == 1)
                            ? OptimizationLevelHandler.get()
                            : GCCOptimizationLevelHandler.get());
    // #pragma noparallel
    NoParallelHandler.reset(new PragmaNoParallelHandler());
    PP.AddPragmaHandler(NoParallelHandler.get());
    // #pragma parallel
    ParallelHandler.reset(new PragmaParallelHandler());
    PP.AddPragmaHandler(ParallelHandler.get());
    // #pragma nounroll
    NoUnrollHandler.reset(new PragmaNoUnrollHandler());
    PP.AddPragmaHandler(NoUnrollHandler.get());
    // #pragma unroll
    UnrollHandler.reset(new PragmaUnrollHandler());
    PP.AddPragmaHandler(UnrollHandler.get());
    // #pragma nounroll_and_jam
    NoUnrollAndJamHandler.reset(new PragmaNoUnrollAndJamHandler());
    PP.AddPragmaHandler(NoUnrollAndJamHandler.get());
    // #pragma unroll_and_jam
    UnrollAndJamHandler.reset(new PragmaUnrollAndJamHandler());
    PP.AddPragmaHandler(UnrollAndJamHandler.get());
    // #pragma nofusion
    NoFusionHandler.reset(new PragmaNoFusionHandler());
    PP.AddPragmaHandler(NoFusionHandler.get());
    // #pragma ident
    IdentHandler.reset(new PragmaIdentHandler());
    PP.AddPragmaHandler(IdentHandler.get());
    // #pragma optimization_parameter
    OptimizationParameterHandler.reset(new PragmaOptimizationParameterHandler());
    PP.AddPragmaHandler("intel", OptimizationParameterHandler.get());
    // #pragma alloc_section
    AllocSectionHandler.reset(new PragmaAllocSectionHandler());
    PP.AddPragmaHandler(AllocSectionHandler.get());
    // #pragma section
    SectionHandler.reset(new PragmaSectionHandler());
    PP.AddPragmaHandler(SectionHandler.get());
    // #pragma alloc_text
    AllocTextHandler.reset(new PragmaAllocTextHandler());
    PP.AddPragmaHandler(AllocTextHandler.get());
    // #pragma auto_inline
    AutoInlineHandler.reset(new PragmaAutoInlineHandler());
    PP.AddPragmaHandler(AutoInlineHandler.get());
    // #pragma bss_seg|code_seg|const_seg|data_seg
    BssSegHandler.reset(new PragmaBssSegHandler());
    PP.AddPragmaHandler(BssSegHandler.get());
    CodeSegHandler.reset(new PragmaCodeSegHandler());
    PP.AddPragmaHandler(CodeSegHandler.get());
    ConstSegHandler.reset(new PragmaConstSegHandler());
    PP.AddPragmaHandler(ConstSegHandler.get());
    DataSegHandler.reset(new PragmaDataSegHandler());
    PP.AddPragmaHandler(DataSegHandler.get());
    // #pragma check_stack
    CheckStackHandler.reset(new PragmaCheckStackHandler());
    PP.AddPragmaHandler(CheckStackHandler.get());
    // #pragma component
    ComponentHandler.reset(new PragmaComponentHandler());
    PP.AddPragmaHandler(ComponentHandler.get());
    // #pragma conform
    ConformHandler.reset(new PragmaConformHandler());
    PP.AddPragmaHandler(ConformHandler.get());
    // #pragma deprecated
    DeprecatedHandler.reset(new PragmaDeprecatedHandler());
    PP.AddPragmaHandler(DeprecatedHandler.get());
    // #pragma fp_contract
    IntelFPContractHandler.reset(new PragmaIntelFPContractHandler());
    PP.AddPragmaHandler(IntelFPContractHandler.get());
    // #pragma fenv_access
    IntelFenvAccessHandler.reset(new PragmaIntelFenvAccessHandler());
    PP.AddPragmaHandler(IntelFenvAccessHandler.get());
    // #pragma init_seg
    InitSegHandler.reset(new PragmaInitSegHandler());
    PP.AddPragmaHandler(InitSegHandler.get());
    // #pragma float_control
    FPState = getLangOpts().getFPModel();
    FCVector.clear();
    FCVector.push_back(FPState);
    FloatControlHandler.reset(new PragmaFloatControlHandler());
    PP.AddPragmaHandler(FloatControlHandler.get());
    // Apply default fp options
    if (FPState != (LangOptions::IFP_Fast | LangOptions::IFP_FP_Contract)) {
      Actions.ActOnPragmaOptionsFloatControl(SourceLocation(), FPState);
      if (FPState == (LangOptions::IFP_Precise | LangOptions::IFP_FEnv_Access |
                      LangOptions::IFP_Except | LangOptions::IFP_ValueSafety)) {
        Actions.ActOnPragmaCommonOnOff(SourceLocation(), "fp_contract",
                                       "FP_CONTRACT", Sema::IntelCommonOff,
                                       Sema::IntelPragmaFPContract, FPState);
        Actions.ActOnPragmaCommonOnOff(SourceLocation(), "fenv_access",
                                       "FENV_ACCESS", Sema::IntelCommonOn,
                                       Sema::IntelPragmaFEnvAccess, FPState);
      }
    }
    // #pragma region
    RegionHandler.reset(new PragmaRegionHandler());
    PP.AddPragmaHandler(RegionHandler.get());
    // #pragma endregion
    EndRegionHandler.reset(new PragmaEndRegionHandler());
    PP.AddPragmaHandler(EndRegionHandler.get());
    // #pragma start_map_region
    PragmaStartMapRegionHandler *hndlr = new PragmaStartMapRegionHandler();
    StartMapRegionHandler.reset(hndlr);
    PP.AddPragmaHandler(StartMapRegionHandler.get());
    // #pragma stop_map_region
    StopMapRegionHandler.reset(
        new PragmaStopMapRegionHandler(hndlr->RegionStarted));
    PP.AddPragmaHandler(StopMapRegionHandler.get());
    // #pragma vtordisp
    VtorDispHandler.reset(new PragmaVtorDispHandler());
    PP.AddPragmaHandler(VtorDispHandler.get());

    if (getLangOpts().AlignMac68k) {
      Actions.SetMac68kAlignment();
    }
  }
#endif // INTEL_SPECIFIC_IL0_BACKEND
}

void Parser::resetIntelPragmaHandlers() {
  // Remove the pragma handlers we installed.
#if INTEL_SPECIFIC_CILKPLUS
  if (getLangOpts().CilkPlus) {
    PP.RemovePragmaHandler(CilkGrainsizeHandler.get());
    CilkGrainsizeHandler.reset();
    PP.RemovePragmaHandler(SIMDHandler.get());
    SIMDHandler.reset(new PragmaSIMDHandler());
  }
#endif // INTEL_SPECIFIC_CILKPLUS

#ifdef INTEL_SPECIFIC_IL0_BACKEND
  if (getLangOpts().IntelCompat) {
    // #pragma ivdep
    PP.RemovePragmaHandler(IvdepHandler.get());
    IvdepHandler.reset();
    // #pragma novector
    PP.RemovePragmaHandler(NoVectorHandler.get());
    NoVectorHandler.reset();
    // #pragma vector
    PP.RemovePragmaHandler(VectorHandler.get());
    VectorHandler.reset();
    // #pragma distribute_point
    PP.RemovePragmaHandler(DistributeHandler.get());
    DistributeHandler.reset();
    PP.RemovePragmaHandler(DistributeHandler1.get());
    DistributeHandler1.reset();
    // #pragma inline
    PP.RemovePragmaHandler(InlineHandler.get());
    InlineHandler.reset();
    // #pragma noinline
    PP.RemovePragmaHandler(NoInlineHandler.get());
    NoInlineHandler.reset();
    // #pragma forceinline
    PP.RemovePragmaHandler(ForceInlineHandler.get());
    ForceInlineHandler.reset();
    // #pragma loop_count
    PP.RemovePragmaHandler(LoopCountHandler.get());
    LoopCountHandler.reset();
    PP.RemovePragmaHandler(LoopCountHandler1.get());
    LoopCountHandler1.reset();
    // #pragma optimize
    PP.RemovePragmaHandler(IntelOptimizeHandler.get());
    IntelOptimizeHandler.reset();
    // #pragma optimization_level
    PP.RemovePragmaHandler((getLangOpts().PragmaOptimizationLevelIntel == 1)
                               ? OptimizationLevelHandler.get()
                               : GCCOptimizationLevelHandler.get());
    PP.RemovePragmaHandler("intel", OptimizationLevelHandler.get());
    OptimizationLevelHandler.reset();
    PP.RemovePragmaHandler("GCC", GCCOptimizationLevelHandler.get());
    GCCOptimizationLevelHandler.reset();
    // #pragma noparallel
    PP.RemovePragmaHandler(NoParallelHandler.get());
    NoParallelHandler.reset();
    // #pragma parallel
    PP.RemovePragmaHandler(ParallelHandler.get());
    ParallelHandler.reset();
    // #pragma nounroll
    PP.RemovePragmaHandler(NoUnrollHandler.get());
    NoUnrollHandler.reset();
    // #pragma unroll
    PP.RemovePragmaHandler(UnrollHandler.get());
    UnrollHandler.reset();
    // #pragma nounroll_and_jam
    PP.RemovePragmaHandler(NoUnrollAndJamHandler.get());
    NoUnrollAndJamHandler.reset();
    // #pragma unroll_and_jam
    PP.RemovePragmaHandler(UnrollAndJamHandler.get());
    UnrollAndJamHandler.reset();
    // #pragma nofusion
    PP.RemovePragmaHandler(NoFusionHandler.get());
    NoFusionHandler.reset();
    // #pragma ident
    PP.RemovePragmaHandler(IdentHandler.get());
    IdentHandler.reset();
    // #pragma optimization_parameter
    PP.RemovePragmaHandler("intel", OptimizationParameterHandler.get());
    OptimizationParameterHandler.reset();
    // #pragma alloc_section
    PP.RemovePragmaHandler(AllocSectionHandler.get());
    AllocSectionHandler.reset();
    // #pragma section
    PP.RemovePragmaHandler(SectionHandler.get());
    SectionHandler.reset();
    // #pragma alloc_text
    PP.RemovePragmaHandler(AllocTextHandler.get());
    AllocTextHandler.reset();
    // #pragma auto_inline
    PP.RemovePragmaHandler(AutoInlineHandler.get());
    AutoInlineHandler.reset();
    // #pragma bss_seg|code_seg|const_seg|data_seg
    PP.RemovePragmaHandler(BssSegHandler.get());
    BssSegHandler.reset();
    PP.RemovePragmaHandler(CodeSegHandler.get());
    CodeSegHandler.reset();
    PP.RemovePragmaHandler(ConstSegHandler.get());
    ConstSegHandler.reset();
    PP.RemovePragmaHandler(DataSegHandler.get());
    DataSegHandler.reset();
    // #pragma check_stack
    PP.RemovePragmaHandler(CheckStackHandler.get());
    CheckStackHandler.reset();
    // #pragma component
    PP.RemovePragmaHandler(ComponentHandler.get());
    ComponentHandler.reset();
    // #pragma conform
    PP.RemovePragmaHandler(ConformHandler.get());
    ConformHandler.reset();
    // #pragma deprecated
    PP.RemovePragmaHandler(DeprecatedHandler.get());
    DeprecatedHandler.reset();
    // #pragma fp_contract
    PP.RemovePragmaHandler(IntelFPContractHandler.get());
    IntelFPContractHandler.reset();
    // #pragma fenv_access
    PP.RemovePragmaHandler(IntelFenvAccessHandler.get());
    IntelFenvAccessHandler.reset();
    // #pragma init_seg
    PP.RemovePragmaHandler(InitSegHandler.get());
    InitSegHandler.reset();
    // #pragma float_control
    FCVector.clear();
    PP.RemovePragmaHandler(FloatControlHandler.get());
    FloatControlHandler.reset();
    // #pragma region
    static_cast<PragmaRegionHandler *>(RegionHandler.get())
        ->CheckOpenedRegions(PP);
    PP.RemovePragmaHandler(RegionHandler.get());
    RegionHandler.reset();
    // #pragma endregion
    PP.RemovePragmaHandler(EndRegionHandler.get());
    EndRegionHandler.reset();
    // #pragma start_map_region
    PP.RemovePragmaHandler(StartMapRegionHandler.get());
    StartMapRegionHandler.reset();
    // #pragma stop_map_region
    PP.RemovePragmaHandler(StopMapRegionHandler.get());
    StopMapRegionHandler.reset();
    // #pragma vtordisp
    PP.RemovePragmaHandler(VtorDispHandler.get());
    VtorDispHandler.reset();
  }
#endif // INTEL_SPECIFIC_IL0_BACKEND
}
