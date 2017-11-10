//===--- DiagnosticIntelParseKinds.td - Intel libparse diagnostics --------===//
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
// Parser Diagnostics
//===----------------------------------------------------------------------===//

let Component = "Parse" in {

let CategoryName = "Parse Issue" in {

// CQ#364426
def warn_empty_enum : Warning<"use of empty enum">, InGroup<IntelCompat>;
// CQ#369185 - support of __bases and __direct_bases intrinsics.
def err_bases_arg_type_not_dependent: Error<"'%select{__bases|__direct_bases}0' "
  "specifier must be used with dependent type">;
def err_expected_ellipsis : Error<"expected '...' after "
  "'%select{__bases|__direct_bases}0' specifier">;
// CQ#364737
def warn_inline_not_allowed : Warning<
  "\"inline\" is not allowed">,
  InGroup<IntelCompat>;
// CQ#370092
def warn_attributes_not_allowed : Warning<
  "an attribute list cannot appear here; attributes are ignored">,
  InGroup<IgnoredAttributes>;
// CQ#373607
def warn_member_function_initialization : Warning<
  "initializer on function does not look like a pure-specifier">,
  InGroup<IntelCompat>;

// if INTEL_SPECIFIC_CILKPLUS
// Cilk Plus
def err_cilkplus_disable: Error<
  "Cilk Plus support disabled - compile with -fcilkplus">;

def err_cilk_for_missing_control_variable: Error<
  "missing control variable declaration and initialization in '_Cilk_for'">;

def err_cilk_for_missing_condition: Error<
  "missing loop condition expression in '_Cilk_for'">;

def err_cilk_for_missing_increment: Error<
  "missing loop increment expression in '_Cilk_for'">;

def err_cilk_for_missing_semi: Error<
  "expected ';' in '_Cilk_for'">;

def err_cilk_for_expect_grainsize: Error<
  "expected 'grainsize' in '#pragma cilk'">;

def err_cilk_for_expect_assign: Error<
  "expected '=' in '#pragma cilk'">;

def warn_cilk_for_following_grainsize: Warning<
  "'#pragma cilk' ignored, because it is not followed by a '_Cilk_for' loop">,
  InGroup<SourceUsesCilkPlus>;

// Cilk elemental functions
def err_elemental_parameter_pack_unsupported: Error<
  "template parameter pack is not supported in '%0' attribute">;

def err_expected_ident_or_this : Error<
  "expected identifier or this">;

def err_expected_ident : Error<
  "expected identifier">;

def err_expected_comma : Error<"expected ','">;
def err_expected_lparen : Error<"expected '('">;
def err_expected_rparen : Error<"expected ')'">;
def err_expected_colon : Error<"expected ':'">;

// Pragma SIMD
def err_simd_for_missing_initialization: Error<
  "missing initialization in simd for">;

def err_simd_for_missing_condition: Error<
  "missing loop condition expression in simd for">;

def err_simd_for_missing_increment: Error<
  "missing loop increment expression in simd for">;

def err_simd_for_missing_semi: Error<
  "expected ';' in simd for">;

def warn_simd_invalid_clause: Warning<"invalid pragma simd clause">,
   InGroup<IntelPragma>;

def err_simd_expected_reduction_operator: Error<
  "expected reduction operator">;
// endif INTEL_SPECIFIC_CILKPLUS
// ifdef INTEL_SPECIFIC_IL0_BACKEND
def x_warn_intel_pragma_malformed : Warning<"unknown pragma qualifier">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_extra_text : Warning<"extra text after expected end of preprocessing directive">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_statement_precede : Warning<"this pragma must immediately precede a statement">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_expected_string : Warning<"expected a string">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_on_off_expected : Warning<"'on' or 'off' expected">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_integer_const : Warning<"expected an integer constant">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_reset_integer_const : Warning<"expected reset or an integer constant">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_no_null_string : Warning<"invalid use of null string; pragma ignored">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_unexpected_section : Warning<"Unexpected section '%0'">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_conform_value : Warning<"forScope behavior is %select{non-standard|standard}0">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_missing_region : Warning<"the #pragma region for this #pragma endregion is missing">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_missing_endregion : Warning<"the #pragma region unclosed at end of file">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_missing_start_map_region : Warning<"no #pragma start_map_region is currently active: pragma ignored">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_missing_stop_map_region : Warning<"#pragma start_map_region already active: pragma ignored">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_assert_before_always : Warning<"always must precede assert qualifier, ignored">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_qualifier_conflict : Warning<"invalid combination of pragma qualifiers">,
  InGroup<IntelPragma>;
def x_error_intel_pragma_statement_precede : Error<"this pragma must immediately precede a statement">;
def x_error_intel_pragma_no_section : Error<"no section is specified in #pragma %0">;
def x_error_intel_pragma_no_variables : Error<"no variables are specified in #pragma %0">;
// endif INTEL_SPECIFIC_IL0_BACKEND
} // end of Parse Issue category.
} // end of Parser diagnostics
