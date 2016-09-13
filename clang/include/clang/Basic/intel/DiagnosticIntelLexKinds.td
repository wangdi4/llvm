//==--- DiagnosticLexKinds.td - liblex diagnostics ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Lexer Diagnostics
//===----------------------------------------------------------------------===//

let Component = "Lex", CategoryName = "Lexical or Preprocessor Issue" in {

// CQ#368194 - emit a warning, not an error on unterminated block comment.
def warn_unterminated_block_comment : Warning<
  err_unterminated_block_comment.Text>, InGroup<Comment>;

// CQ#369184
def err_decimal_ext_unsupported : Error<
  "GNU decimal type extension not supported">;

// CQ#372653.
def warn_pp_malformed_ident :
  Warning<"invalid #ident directive will be ignored">,
  InGroup<IntelCompat>;

// CQ#367740
def warn_pragma_message_malformed : Warning<
  "pragma %select{message|warning|error}0 requires parenthesized string">,
  InGroup<IntelCompat>;

def warn_too_few_args_in_macro_invoc : Warning<
  "too few arguments provided to function-like macro invocation">,
  InGroup<IntelCompat>;

def warn_missing_fnmacro_args_replaced_by_empty : Warning<
  "%0 missing macro argument%s0 treated as empty string%s0">,
  InGroup<IntelCompat>;

def warn_pp_line_invalid_filename : Warning<
  "expected a file name">,
  InGroup<UnknownPragmas>;

def err_bad_import_value : Error<
  "bad attribute value in #import directive">;

def err_typelib_not_found : Error<
  "could not determine header file for this typelib">;

def err_import_exec : Error<
  "could not process #import directive with %0">;
}
