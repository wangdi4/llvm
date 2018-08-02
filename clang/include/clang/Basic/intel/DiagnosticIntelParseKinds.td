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

def warn_pragma_expected_value : Warning<
  "expected value with '#pragma %0' - ignored">,
  InGroup<IgnoredPragmas>;
def warn_pragma_expected_safelen_array_ivdep_clause : Warning<
  "expected safelen or array value with '#pragma ivdep' - ignored">,
  InGroup<IgnoredPragmas>;
def warn_pragma_expected_loop_back_ivdep_clause : Warning<
  "expected loop or back with '#pragma ivdep' - ignored">,
  InGroup<IgnoredPragmas>;
def warn_multiple_ivdep_clause : Warning<
  "%select{'safelen'|'array'|'loop'|'back'}0 cannot appear "
  "multiple times in '#pragma ivdep' - ignored">,
  InGroup<IgnoredPragmas>;
def warn_invalid_ivdep_combination : Warning<
  "%select{'loop'|'back'}0 may not appear in '#pragma ivdep' here - ignored">,
  InGroup<IgnoredPragmas>;
def warn_pragma_vector_invalid_option : Warning<
  "%select{invalid|missing}0 option%select{ %1|}0; expected always - ignored">,
  InGroup<IgnoredPragmas>;
def warn_pragma_block_loop_invalid_option : Warning<
  "invalid option%select{ %1|}0; expected factor, level or private - ignored">,
  InGroup<IgnoredPragmas>;
def warn_multiple_blockloop_clause : Warning<
  "%select{'factor'|'level'|'private'}0 cannot appear "
  "multiple times in '#pragma block_loop' - ignored">,
  InGroup<IgnoredPragmas>;

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
} // end of Parse Issue category.
} // end of Parser diagnostics
