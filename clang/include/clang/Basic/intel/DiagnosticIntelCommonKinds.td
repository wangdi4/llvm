//==--- DiagnosticCommonKinds.td - common diagnostics ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Common Helpers
//===----------------------------------------------------------------------===//

let Component = "Common" in {

let CategoryName = "Parse Issue" in {

// CQ#370092
// if INTEL_CUSTOMIZATION
def warn_attribute_not_type_attr : Warning<
  "%0 attribute cannot be applied to types">,
  InGroup<IgnoredAttributes>;
def err_pragma_simd_expected_for_loop : Error<
  "expected for statement following '#pragma simd'">;
def err_pragma_simd_invalid_linear_var : Error<
  "invalid linear variable">;
// endif INTEL_CUSTOMIZATION
}

let CategoryName = "Nullability Issue" in {

// ifdef INTEL_SPECIFIC_IL0_BACKEND
// Parse && Sema
def x_warn_intel_pragma_wrong_place : Warning<"this pragma may not be used here">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_func_expect : Warning<"expected a function name">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_expected_common : Warning<"%0 %select{is|are}1 expected">,
  InGroup<IntelPragma>;
def x_error_intel_pragma_wrong_place : Error<"this pragma may not be used here">;
def x_error_intel_pragma_loop_count : Error<"invalid loop count">;
def x_error_intel_pragma_declaration_precede : Error<"this pragma must immediately precede a declaration">;
def x_error_intel_pragma_invalid_parallel_pragma : Error<"invalid parallel pragma">;
def x_error_intel_pragma_parallel_vars : Error<"only simple variables or array expressions are allowed in #pragma parallel">;
def x_error_intel_pragma_parallel_private : Error<"variable %0 in firstprivate or lastprivate clause may not be in private clause">;
def x_error_intel_pragma_expected_common : Error<"%0 %select{is|are}1 expected">;
// endif INTEL_SPECIFIC_IL0_BACKEND
}

}