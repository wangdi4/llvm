//===--- DiagnosticIntelLexKinds.td - Intel liblex diagnostics ------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Lexer Diagnostics
//===----------------------------------------------------------------------===//

let Component = "Lex", CategoryName = "Lexical or Preprocessor Issue" in {

// CQ#367740
def warn_pragma_message_malformed : Warning<
  "pragma %select{message|warning|error}0 requires parenthesized string">,
  InGroup<IntelCompat>;

// IntelCompat: AllowFewerMacroArgs
def warn_too_few_args_in_macro_invoc : Warning<
  "too few arguments provided to function-like macro invocation">,
  InGroup<IntelCompat>;
def warn_missing_fnmacro_args_replaced_by_empty : Warning<
  "%0 missing macro argument%s0 treated as empty string%s0">,
  InGroup<IntelCompat>;

def err_bad_import_value : Error<
  "bad attribute value in #import directive">;

def err_typelib_not_found : Error<
  "could not determine header file for this typelib">;

def err_import_exec : Error<
  "could not process #import directive with %0">;
}
