//===--- DiagnosticIntelParseKinds.td - Intel libparse diagnostics --------===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
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

// Warn if '#pragma ivdep' is used for FPGA emulator
def warn_pragma_ivdep_is_used_for_emulator : Warning<
  "'#pragma ivdep' is used in FPGA emulator environment\n"
  "NOTE: incorrect use of the ivdep pragma can result in "
  "functional issues that will not be replicated in the emulator flow">,
  InGroup<IvdepFPGAUsage>;

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
def warn_pragma_loop_count_invalid_option
    : Warning<"invalid loop count '#pragma loop_count' - ignored">,
      InGroup<IgnoredPragmas>;

// OpenMP
def warn_pragma_omp_ignored_for_target : Warning<
  "OpenMP directive '%0' ignored for target '%1'">,
  InGroup<SourceUsesOpenMP>, DefaultIgnore;

def err_omp_decl_in_declare_variant : Error<
  "function declaration is expected after 'declare variant' directive">;
def err_omp_declare_variant_not_found : Error<
  "variant function not found">;
def err_omp_declare_variant_not_a_function : Error<
  "variant must be a function">;
def err_omp_declare_variant_func_id_expected : Error<
  "variant must be the name of a function or a template-id">;
def err_omp_missing_match_clause : Error<
  "omp declare variant requires a match clause">;
def err_omp_bad_context_selector_set : Error<
  "unknown or unsupported context selector set, expecting %0">;
def err_omp_bad_context_selector : Error<
  "unknown or unsupported '%0' context selector, expecting %1">;
def err_omp_bad_device_selector : Error<
 "unknown or unsupported '%0' selector, expecting %1">;

// CQ#364426
def warn_empty_enum : Warning<"use of empty enum">, InGroup<IntelCompat>;
def err_expected_ellipsis : Error<"expected '...' after "
  "'%select{__bases|__direct_bases}0' specifier">;
// CQ#364737
def warn_inline_not_allowed : Warning<
  "\"inline\" is not allowed">,
  InGroup<IntelCompat>;
// CQ#373607
def warn_member_function_initialization : Warning<
  "initializer on function does not look like a pure-specifier">,
  InGroup<IntelCompat>;
} // end of Parse Issue category.
} // end of Parser diagnostics
