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

def err_no_arith_type : Error <"expression must have an arithmetic type">;

def warn_attributes_ignored_after_init
    : Warning<"variable attributes appearing after a parenthesized initializer "
              "are ignored">,
      InGroup<IgnoredAttributes>;

// CQ#410305 ignore declare target directive inside function.
def warn_omp_ignore_unexpected_directive : Warning<
  "ignoring unexpected OpenMP directive %select{|'#pragma omp %1'}0">,
  InGroup<IntelCompat>;

def err_expected_three : Error<"expected %0, %1 or %2">;
def err_omp_unknown_lastprivate_modifier : Error<
  "incorrect lastprivate modifier, expected 'conditional'">;
def err_omp_expected_identifier_for_ompx_processor : Error<
  "expected identifier in 'ompx_processor' clause">;

#if INTEL_FEATURE_CSA
def err_omp_unknown_dataflow_modifier : Error<
  "unexpected 'dataflow' modifier, expected 'static', 'num_workers'"
  " or 'pipeline'">;
def err_omp_conflicting_dataflow_num_workers_modifier: Error<
  "'num_workers' modifier of 'dataflow' clause conflicts with 'num_threads'"
  " clause">;
def note_omp_conflicting_dataflow_num_workers_modifier: Note<
  "conflicting entry is here">;
#endif // INTEL_FEATURE_CSA

// Warn if '#pragma ivdep' is used for FPGA emulator
def warn_pragma_ivdep_is_used_for_emulator : Warning<
  "'#pragma ivdep' is used in FPGA emulator environment\n"
  "NOTE: incorrect use of the ivdep pragma can result in "
  "functional issues that will not be replicated in the emulator flow">,
  InGroup<IvdepFPGAUsage>;

def warn_pragma_expected_value : Warning<
  "expected value with '#pragma %0' - ignored">,
  InGroup<IgnoredPragmas>;
def warn_pragma_expected_depth_independent_loop_fuse_clause : Warning<
  "Invalid clause. Expected independent or depth value with '#pragma loop_fuse'"
  " - ignored">,
  InGroup<IgnoredPragmas>;
def warn_multiple_loop_fuse_clause : Warning<
  "%select{'depth'|'independent'}0 cannot appear "
  "multiple times in '#pragma loop_fuse' - ignored">,
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
def warn_pragma_vector_invalid_option
    : Warning<"%select{invalid|missing}0 option%select{ %1|}0; "
              "%select{expected always|always must precede assert qualifier}0 "
              "- ignored">,
      InGroup<IgnoredPragmas>;
def warn_pragma_vector_conflicting_aligned_unaligned
    : Warning<"%select{'aligned'|'unaligned'}0 clause conflicts with "
              "%select{'unaligned'|'aligned'}0 clause">,
      InGroup<IgnoredPragmas>;
def note_pragma_vector_conflicting_aligned_unaligned
    : Note<"%select{'aligned'|'unaligned'}0 is used here">;
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
def err_pragma_expected_non_negative_integer_const
    : Error<"expected non-negative integer constant in '#pragma %0'">;
def warn_pragma_unsupported_modifier
    : Warning<"unsupported modifier '%0' - ignored">,
      InGroup<IgnoredPragmas>;

// OpenMP
def warn_pragma_omp_ignored_for_target : Warning<
  "OpenMP directive '%0' ignored for target '%1'">,
  InGroup<SourceUsesOpenMP>, DefaultIgnore;
def warn_pragma_omp_unimplemented : Warning<
  "OpenMP directive '%0' unimplemented - ignored">,
  InGroup<IgnoredPragmas>;
def err_pragma_omp_expected_comma  : Error<
  "Expected comma in '%0' clause">;

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
