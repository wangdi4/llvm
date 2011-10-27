//===--- ParsePragma.cpp - Language specific pragma parsing ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the language specific #pragma handlers.
//
//===----------------------------------------------------------------------===//

#include "ParsePragma.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Parse/ParseDiagnostic.h"
#include "clang/Parse/Parser.h"
#include "clang/Lex/Preprocessor.h"
using namespace clang;


// #pragma GCC visibility comes in two variants:
//   'push' '(' [visibility] ')'
//   'pop'
void PragmaGCCVisibilityHandler::HandlePragma(Preprocessor &PP, Token &VisTok) {
  SourceLocation VisLoc = VisTok.getLocation();

  Token Tok;
  PP.Lex(Tok);

  const IdentifierInfo *PushPop = Tok.getIdentifierInfo();

  bool IsPush;
  const IdentifierInfo *VisType;
  if (PushPop && PushPop->isStr("pop")) {
    IsPush = false;
    VisType = 0;
  } else if (PushPop && PushPop->isStr("push")) {
    IsPush = true;
    PP.Lex(Tok);
    if (Tok.isNot(tok::l_paren)) {
      PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_lparen)
        << "visibility";
      return;
    }
    PP.Lex(Tok);
    VisType = Tok.getIdentifierInfo();
    if (!VisType) {
      PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_identifier)
        << "visibility";
      return;
    }
    PP.Lex(Tok);
    if (Tok.isNot(tok::r_paren)) {
      PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_rparen)
        << "visibility";
      return;
    }
  } else {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_identifier)
      << "visibility";
    return;
  }
  PP.Lex(Tok);
  if (Tok.isNot(tok::eom)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol)
      << "visibility";
    return;
  }

  Actions.ActOnPragmaVisibility(IsPush, VisType, VisLoc);
}

// #pragma pack(...) comes in the following delicious flavors:
//   pack '(' [integer] ')'
//   pack '(' 'show' ')'
//   pack '(' ('push' | 'pop') [',' identifier] [, integer] ')'
void PragmaPackHandler::HandlePragma(Preprocessor &PP, Token &PackTok) {
  SourceLocation PackLoc = PackTok.getLocation();

  Token Tok;
  PP.Lex(Tok);
  if (Tok.isNot(tok::l_paren)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_lparen) << "pack";
    return;
  }

  Sema::PragmaPackKind Kind = Sema::PPK_Default;
  IdentifierInfo *Name = 0;
  ExprResult Alignment;
  SourceLocation LParenLoc = Tok.getLocation();
  PP.Lex(Tok);
  if (Tok.is(tok::numeric_constant)) {
    Alignment = Actions.ActOnNumericConstant(Tok);
    if (Alignment.isInvalid())
      return;

    PP.Lex(Tok);
  } else if (Tok.is(tok::identifier)) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    if (II->isStr("show")) {
      Kind = Sema::PPK_Show;
      PP.Lex(Tok);
    } else {
      if (II->isStr("push")) {
        Kind = Sema::PPK_Push;
      } else if (II->isStr("pop")) {
        Kind = Sema::PPK_Pop;
      } else {
        PP.Diag(Tok.getLocation(), diag::warn_pragma_pack_invalid_action);
        return;
      }
      PP.Lex(Tok);

      if (Tok.is(tok::comma)) {
        PP.Lex(Tok);

        if (Tok.is(tok::numeric_constant)) {
          Alignment = Actions.ActOnNumericConstant(Tok);
          if (Alignment.isInvalid())
            return;

          PP.Lex(Tok);
        } else if (Tok.is(tok::identifier)) {
          Name = Tok.getIdentifierInfo();
          PP.Lex(Tok);

          if (Tok.is(tok::comma)) {
            PP.Lex(Tok);

            if (Tok.isNot(tok::numeric_constant)) {
              PP.Diag(Tok.getLocation(), diag::warn_pragma_pack_malformed);
              return;
            }

            Alignment = Actions.ActOnNumericConstant(Tok);
            if (Alignment.isInvalid())
              return;

            PP.Lex(Tok);
          }
        } else {
          PP.Diag(Tok.getLocation(), diag::warn_pragma_pack_malformed);
          return;
        }
      }
    }
  }

  if (Tok.isNot(tok::r_paren)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_rparen) << "pack";
    return;
  }

  SourceLocation RParenLoc = Tok.getLocation();
  PP.Lex(Tok);
  if (Tok.isNot(tok::eom)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol) << "pack";
    return;
  }

  Actions.ActOnPragmaPack(Kind, Name, Alignment.release(), PackLoc,
                          LParenLoc, RParenLoc);
}

// #pragma 'align' '=' {'native','natural','mac68k','power','reset'}
// #pragma 'options 'align' '=' {'native','natural','mac68k','power','reset'}
static void ParseAlignPragma(Sema &Actions, Preprocessor &PP, Token &FirstTok,
                             bool IsOptions) {
  Token Tok;

  if (IsOptions) {
    PP.Lex(Tok);
    if (Tok.isNot(tok::identifier) ||
        !Tok.getIdentifierInfo()->isStr("align")) {
      PP.Diag(Tok.getLocation(), diag::warn_pragma_options_expected_align);
      return;
    }
  }

  PP.Lex(Tok);
  if (Tok.isNot(tok::equal)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_align_expected_equal)
      << IsOptions;
    return;
  }

  PP.Lex(Tok);
  if (Tok.isNot(tok::identifier)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_identifier)
      << (IsOptions ? "options" : "align");
    return;
  }

  Sema::PragmaOptionsAlignKind Kind = Sema::POAK_Natural;
  const IdentifierInfo *II = Tok.getIdentifierInfo();
  if (II->isStr("native"))
    Kind = Sema::POAK_Native;
  else if (II->isStr("natural"))
    Kind = Sema::POAK_Natural;
  else if (II->isStr("packed"))
    Kind = Sema::POAK_Packed;
  else if (II->isStr("power"))
    Kind = Sema::POAK_Power;
  else if (II->isStr("mac68k"))
    Kind = Sema::POAK_Mac68k;
  else if (II->isStr("reset"))
    Kind = Sema::POAK_Reset;
  else {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_align_invalid_option)
      << IsOptions;
    return;
  }

  SourceLocation KindLoc = Tok.getLocation();
  PP.Lex(Tok);
  if (Tok.isNot(tok::eom)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol)
      << (IsOptions ? "options" : "align");
    return;
  }

  Actions.ActOnPragmaOptionsAlign(Kind, FirstTok.getLocation(), KindLoc);
}

void PragmaAlignHandler::HandlePragma(Preprocessor &PP, Token &AlignTok) {
  ParseAlignPragma(Actions, PP, AlignTok, /*IsOptions=*/false);
}

void PragmaOptionsHandler::HandlePragma(Preprocessor &PP, Token &OptionsTok) {
  ParseAlignPragma(Actions, PP, OptionsTok, /*IsOptions=*/true);
}

// #pragma unused(identifier)
void PragmaUnusedHandler::HandlePragma(Preprocessor &PP, Token &UnusedTok) {
  // FIXME: Should we be expanding macros here? My guess is no.
  SourceLocation UnusedLoc = UnusedTok.getLocation();

  // Lex the left '('.
  Token Tok;
  PP.Lex(Tok);
  if (Tok.isNot(tok::l_paren)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_lparen) << "unused";
    return;
  }
  SourceLocation LParenLoc = Tok.getLocation();

  // Lex the declaration reference(s).
  llvm::SmallVector<Token, 5> Identifiers;
  SourceLocation RParenLoc;
  bool LexID = true;

  while (true) {
    PP.Lex(Tok);

    if (LexID) {
      if (Tok.is(tok::identifier)) {
        Identifiers.push_back(Tok);
        LexID = false;
        continue;
      }

      // Illegal token!
      PP.Diag(Tok.getLocation(), diag::warn_pragma_unused_expected_var);
      return;
    }

    // We are execting a ')' or a ','.
    if (Tok.is(tok::comma)) {
      LexID = true;
      continue;
    }

    if (Tok.is(tok::r_paren)) {
      RParenLoc = Tok.getLocation();
      break;
    }

    // Illegal token!
    PP.Diag(Tok.getLocation(), diag::warn_pragma_unused_expected_punc);
    return;
  }

  PP.Lex(Tok);
  if (Tok.isNot(tok::eom)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol) <<
        "unused";
    return;
  }

  // Verify that we have a location for the right parenthesis.
  LLVM_ASSERT(RParenLoc.isValid() && "Valid '#pragma unused' must have ')'");
  LLVM_ASSERT(!Identifiers.empty() && "Valid '#pragma unused' must have arguments");

  // Perform the action to handle the pragma.
  Actions.ActOnPragmaUnused(Identifiers.data(), Identifiers.size(),
                            parser.getCurScope(), UnusedLoc, LParenLoc, RParenLoc);
}

// #pragma weak identifier
// #pragma weak identifier '=' identifier
void PragmaWeakHandler::HandlePragma(Preprocessor &PP, Token &WeakTok) {
  // FIXME: Should we be expanding macros here? My guess is no.
  SourceLocation WeakLoc = WeakTok.getLocation();

  Token Tok;
  PP.Lex(Tok);
  if (Tok.isNot(tok::identifier)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_identifier) << "weak";
    return;
  }

  IdentifierInfo *WeakName = Tok.getIdentifierInfo(), *AliasName = 0;
  SourceLocation WeakNameLoc = Tok.getLocation(), AliasNameLoc;

  PP.Lex(Tok);
  if (Tok.is(tok::equal)) {
    PP.Lex(Tok);
    if (Tok.isNot(tok::identifier)) {
      PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_identifier)
          << "weak";
      return;
    }
    AliasName = Tok.getIdentifierInfo();
    AliasNameLoc = Tok.getLocation();
    PP.Lex(Tok);
  }

  if (Tok.isNot(tok::eom)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol) << "weak";
    return;
  }

  if (AliasName) {
    Actions.ActOnPragmaWeakAlias(WeakName, AliasName, WeakLoc, WeakNameLoc,
                                 AliasNameLoc);
  } else {
    Actions.ActOnPragmaWeakID(WeakName, WeakLoc, WeakNameLoc);
  }
}

static void ParseFPContractPragma(Sema &Actions, Preprocessor &PP, const Token& T1) {
  if (IdentifierInfo *op = T1.getIdentifierInfo()) {
    if (op->isStr("ON") || 
       (Actions.getLangOptions().OpenCL && op->isStr("DEFAULT"))) {
      // FP_CONTRACT is ON
      Actions.getFPOptions().fp_contract = 1;
      return;
    }
    if (op->isStr("OFF")) {
      // FP_CONTRACT is OFF
      Actions.getFPOptions().fp_contract = 0;
      return;
    }
  }
  PP.Diag(T1.getLocation(), diag::warn_pragma_expected_identifier) << 
    "FP_CONTRACT";
}

void
PragmaOpenCLFPContractHandler::HandlePragma(Preprocessor &PP, Token &CLTok) {
  SourceLocation CLLoc = CLTok.getLocation();

  Token Tok;
  PP.Lex(Tok);
  if (Tok.isNot(tok::identifier)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_identifier)
      << "OPENCL";
    return;
  }

  ParseFPContractPragma(Actions, PP, Tok);
  return;


}

void 
PragmaOpenCLExtensionHandler::HandlePragma(Preprocessor &PP, Token &CLTok) {
  SourceLocation CLLoc = CLTok.getLocation();
  
  Token Tok;  Token EN;

  PP.Lex(EN);
  if (EN.isNot(tok::identifier)) {
    PP.Diag(EN.getLocation(), diag::warn_pragma_expected_identifier) <<
      "OPENCL";
    return;
  }
  IdentifierInfo *ename = EN.getIdentifierInfo();
  PP.Lex(Tok);
  if (Tok.isNot(tok::colon)) {
    PP.Diag(EN.getLocation(), diag::warn_pragma_expected_colon) <<
      ename;
    return;
  }

  Token T1;
  PP.Lex(T1);
  if (T1.isNot(tok::identifier)) {
    PP.Diag(T1.getLocation(), diag::warn_pragma_expected_identifier)
       << "OPENCL";
    return;
  }

  int on = 0;
  if (IdentifierInfo *op = T1.getIdentifierInfo()) {
    if (op->isStr("enable")) {
      on = 1;
    } else if (op->isStr("disable")) {
      on = 0;
    } else {
      PP.Diag(T1.getLocation(), diag::warn_pragma_expected_enable_disable);
      return;
	}
  }

  
  OpenCLOptions &f = Actions.getOpenCLOptions();
  if (ename->isStr("all")) {
#define OPENCLEXT(nm) \
    if (PP.getTargetInfo().isOpenCLExtensionSupported(#nm)) { \
      f.nm = on; \
    }
#include "clang/Basic/OpenCLExtensions.def"
  }
#define OPENCLEXT(nm) \
  else if (ename->isStr(#nm)) { \
    if (PP.getTargetInfo().isOpenCLExtensionSupported(#nm)) { \
      f.nm = on; \
    } else { \
      PP.Diag(T1.getLocation(), diag::warn_extension_not_supported) << ename; \
      return; \
    } \
  }
#include "clang/Basic/OpenCLExtensions.def"
  else {
    PP.Diag(T1.getLocation(), diag::warn_pragma_unknown_extension) << ename;
    return;
  }
}
