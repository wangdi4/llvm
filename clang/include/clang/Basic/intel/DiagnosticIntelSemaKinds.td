//===--- DiagnosticIntelSemaKinds.td - Intel libsema diagnostics ----------===//
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
// Semantic Analysis
//===----------------------------------------------------------------------===//

let Component = "Sema" in {
let CategoryName = "Semantic Issue" in {
// CQ#364268
def warn_main_arg_wrong : Warning<"%select{first|second|third|fourth}0 "
    "parameter of 'main' (%select{argument count|argument array|environment|"
    "platform-specific data}0) should be of type %1">, InGroup<Main>;
// CQ#366562 - allow #pragma unroll with non-positive or too large argument.
def warn_pragma_unroll_invalid_factor_ignored : Warning<
  "unrolling factor '%0' is %select{negative|too large}1 and will be ignored">,
  InGroup<Pragmas>;
def warn_access_dtor_vbase :
    Warning<"inherited virtual base class %1 has "
    "%select{private|protected}2 destructor">,
    AccessControl, InGroup<PrivateVBaseDtor>;
def warn_lvalue_reference_bind_to_initlist : Warning<
  "%select{non-const|volatile}0 lvalue reference to type %1 cannot bind to an "
  "initializer list temporary">, InGroup<IntelCompat>;
def warn_lvalue_reference_bind_to_temporary : Warning<
  "%select{non-const|volatile}0 lvalue reference %diff{to type $ cannot bind "
  "to a temporary of type $|cannot bind to incompatible temporary}1,2">,
  InGroup<IntelCompat>;
// CQ#375011
def err_alloc_size_attribute_argument_not_int : Error<
  "'alloc_size' attribute requires integer constant">;
def warn_attribute_requires_integer_param_type : Warning<
  "%0 attribute requires an integer function parameter">,
  InGroup<IgnoredAttributes>;
def note_param_index_specified_here : Note<
  "parameter's index specified here">;
def err_recursive_attribute_expected : Error<
  "recursive keyword expected after a pragma inline directive">;
// CQ#366612
def warn_func_redecl_conflicting_types: Warning<
  "declaration of %0 is incompatible with previous "
  "%select{declaration|definition}1">, InGroup<IntelCompat>;
  // CQ#366309 - allow arrays with incomplete element type in IntelCompat mode.
def ext_intel_array_incomplete_type : Extension<
  "array with incomplete element type %0 is an Intel extension">,
  InGroup<IntelCompat>;
  // CQ#380872 - Arrays with incomplete (unknown) size
def warn_intel_array_incomplete_size : Warning<
  "array with incomplete (unknown) size element %0">,
  InGroup<IntelCompat>;
// CQ#366312
def ext_intel_lvalue_cast_not_lengthened: ExtWarn<
  "assignment to lvalue cast that does not lengthen the size of "
  "an object is an Intel Extension">, InGroup<IntelCompat>;
// CQ#364630
def ext_intel_typecheck_call_too_many_args : ExtWarn<
  "too many %select{|||execution configuration }0arguments to "
  "%select{function|block|method|kernel function}0 call, "
  "expected %1, have %2">, InGroup<IntelCompat>;
// CQ#364256. Same as warn_return_missing_expr, but not a DefaultError.
def warn_return_missing_expr_no_err : Warning<
  "non-void %select{function|method}1 %0 should return a value">,
  InGroup<IntelCompat>;
// CQ#371990 - let argument be out of Low..High range, assuming Low.
def warn_arg_invalid_range_low_assumed : Warning<
  "expected a value between %0 and %1, defaults to %0">,
  InGroup<IntelCompat>;
// CQ#371340
def warn_non_asm_stmt_in_naked_function : Warning<
  "attribute 'naked' ignored on function with non-ASM statements">,
  InGroup<IgnoredAttributes>;
def err_attribute_argument_is_not_in_range : Error<
  "invalid attribute value, possible values are: %0">;

def warn_bad_cxx_cast_generic : Warning<
  "%select{const_cast|static_cast|reinterpret_cast|dynamic_cast|C-style cast|"
  "functional-style cast}0 from %1 to %2 is allowed in Intel/GCC compatibility mode only">,
  InGroup<IntelCompat>;
def warn_bad_cxx_cast_unrelated_class : Warning<
  "%select{const_cast|static_cast|reinterpret_cast|dynamic_cast|C-style cast|"
  "functional-style cast}0 from %1 to %2, which are not related by "
  "inheritance, is allowed in Intel/GCC compatibility mode only">, InGroup<IntelCompat>;
def err_blocklevel_overlap : Error<
  "overlapping blockloop levels between level(%0:%1) and level(%2:%3)">;
def note_second_block_loop_level_specified_here : Note<
  "second block_loop level is specified here">;
def err_invalid_blocklevel : Error<
  "invalid block level: the first parameter must be less than the second">;
def err_invalid_blocklevel_range : Error<
  "invalid block level: value must be %select{greater than zero|less than or equal to 8}0">;
def err_duplicate_variable_name : Error<
    "duplicate use of variable name \"%0\"">;
def err_pragma_loop_invalid_argument : Error<
  "invalid argument of type %0; expected a scalar type">;
def err_pragma_block_loop_private_expected_var_arg : Error<
  "only variables can be arguments to '#pragma block_loop private'">;
// CMPLRLLVM-1125 - Emit warnings about redundant #pragma ivdeps.
def warn_redundant_ivdep_pragma : Warning<
  "redundant directive '%0'">,
  InGroup<Pragmas>;
def note_overriding_ivdep_pragma_is_specified_here : Note<
  "overriding ivdep pragma is specified here">;
}

def warn_ovl_diff_return_type : Warning<
  "functions that differ only in their return type cannot be overloaded, new definition ignored">, InGroup<IntelCompat>;
def err_va_pack_used_in_non_variadic_function
    : Error<"'%select{__builtin_va_arg_pack()|__builtin_va_arg_pack_len()}0' "
            "used in function with fixed args">;
def err_va_arg_pack_invalid_usage
    : Error<"invalid use of "
            "'%select{__builtin_va_arg_pack()|__builtin_va_arg_pack_len()}0', "
            "must be in an inlined "
            "function that is not externally visible">;
def err_va_pack_not_last : Error<"__builtin_va_arg_pack must be the last "
                                 "parameter in a function call">;
def err_va_pack_not_variadic : Error<"__builtin_va_arg_pack can only be used "
                                     "as a parameter to a variadic argument">;

// Intel HLS extensions
def err_component_func_attr :
  Error<"attribute %0 can only be applied to a component function">;
def err_attribute_interface_invalid_type : Error<
  "string passed to %0 attribute is not a valid interface type. Valid component invocation interface types are: %1">;
def err_hls_merge_dir_invalid : Error<
  "merge direction must be 'depth' or 'width'">;
def err_hls_memory_arg_invalid : Error<
  "%0 attribute requires either no argument or one of: %1">;
def err_bankbits_numbanks_conflicting : Error<
  "the number of bank_bits must be equal to ceil(log2(numbanks))">;
def err_bankbits_non_consecutive : Error<
  "bank_bits must be consecutive">;
def err_memory_attribute_invalid : Error<
  "%0 attribute only applies to "
  "%select{|slave memory arguments, non-static field members, }1"
  "constant variables, local variables and static variables">;
def err_hls_force_loop_pipelining_invalid : Error<
  "%0 attribute argument must be 'on' or 'off'">;
def err_hls_force_loop_pipelining_conflict : Error<
  "%0 and 'hls_force_loop_pipelining(off)' attributes are not compatible">;

// Intel OpenCL FPGA extension
def warn_opencl_attribute_only_for_local_address_space : Warning<
  "%0 attribute can be aplied only for parameters that reside "
  "in local address space, attribute ignored">,
  InGroup<IgnoredAttributes>;
def warn_opencl_attribute_only_for_global_address_space : Warning<
  "%0 attribute can be aplied only for parameters that reside "
  "in global address space, attribute ignored">,
  InGroup<IgnoredAttributes>;
def err_opencl_attributes_conflict : Error<
  "%0 attribute conflicts with '%1' attribute">;
def err_opencl_missing_actual_channel_type : Error<
  "missing actual type specifier for channel">;
def err_intel_opencl_attribute_argument_type : Error<
  "%0 attribute requires %select{int or bool|an integer "
  "constant|a string|an identifier|an unsigned integer}1">;
def err_opencl_attribute_requires_another_to_be_specified : Error<
  "%0 attribute requires %1 to be specified">;
def err_opencl_autorun_kernel_wrong_reqd_wg_size : Error<
  "Autorun kernel functions must have work group sizes that are divisors "
  "of 2^32">;
def err_opencl_autorun_kernel_cannot_have_arguments : Error<
  "Autorun kernel functions cannot have any arguments">;
def err_opencl_x_y_z_arguments_must_be_one : Error<
  "%0 X-, Y- and Z- sizes must be 1 when %1 attribute is used and equals to 0">;
def err_intel_attribute_argument_is_not_in_range: Error<
  "%0 attribute must be in range from 0 to 3">;
def err_intel_opencl_attribute_requires_extension : Error<
  "%0 attribute requires %1 extension to be enabled">;
def warn_intel_opencl_attribute_wrong_decl_type : Warning<
  "%0 attribute only applies to %select{"
  "functions"
  "|unions"
  "|variables and functions"
  "|functions and global variables"
  "|functions, variables, and Objective-C interfaces"
  "|functions and methods"
  "|parameters"
  "|functions, methods and blocks"
  "|functions, methods, and classes"
  "|functions, methods, and parameters"
  "|functions, methods, and global variables"
  "|classes"
  "|enums"
  "|variables"
  "|methods"
  "|fields and global variables"
  "|structs"
  "|parameters and typedefs"
  "|variables and typedefs"
  "|thread-local variables"
  "|variables and fields"
  "|variables, data members and tag types"
  "|types and namespaces"
  "|Objective-C interfaces"
  "|methods and properties"
  "|struct or union"
  "|struct, union or class"
  "|types"
  "|Objective-C instance methods"
  "|init methods of interface or class extension declarations"
  "|variables, functions and classes"
  "|functions, variables, classes, and Objective-C interfaces"
  "|Objective-C protocols"
  "|variables with static or thread storage duration"
  "|functions, methods, properties, and global variables"
  "|structs, unions, and typedefs"
  "|structs and typedefs"
  "|interface or protocol declarations"
  "|kernel functions"
  "|non-K&R-style functions"
  "|variables, enums, fields and typedefs"
  "|functions, methods, enums, and classes"
  "|structs, classes, variables, functions, and inline namespaces"
  "|variables, functions, methods, types, enumerations, enumerators, labels, and non-static data members"
  "|classes and enumerations"
  "|named declarations"
  "|OpenCL channels or pipes"
  "|OpenCL pipes}1">,
  InGroup<IgnoredAttributes>;
def err_opencl_attribute_argument_out_of_bounds : Error<
  "%0 attribute parameter %1 requires integer constant between %2 and %3 "
  "inclusive">;
def err_opencl_power_of_two_in_range : Error<
  "%0 attribute must be a constant power of two between %1 and %2 inclusive">;

def warn_channel_is_used_from_more_than_one_kernel : Warning<
  "usage of the same channel for %select{reading|writing}0 from different "
  "kernels is prohibited">, InGroup<FPGAChannelsUsage>, DefaultError;
def note_channel_usage_already_found_in_kernel : Note<
  "%select{previous|}0 usage of channel '%1' for "
  "%select{reading|writing}2 found in the kernel '%3'. See call stack below:">;
def note_channel_is_used_through : Note<
  "channel '%0' is used for %select{reading|writing}1 here">;
def warn_io_channel_is_used_from_autorun_kernel : Warning<
  "usage of I/O channel '%0' found in autorun kernel '%1'. "
  "See call stack below:">, InGroup<FPGAChannelsUsage>, DefaultError;

def err_hls_builtin_arg_mismatch : Error<
  "HLS builtin parameter must be %select{"
  "a pointer to a complete object type"
  "|an integer"
  "|a non-negative integer constant"
  "|a factor of the type size"
  "|a boolean value"
  "|a pointer to a boolean"
  "|a power of two integer between 8 and 1024"
  "|an integer between 1 and 64 inclusive"
  "|an integer between 1 and 1024 inclusive"
  "|a pointer to an integer"
  "|a non-negative integer constant, 0 or -1}0">;

def err_hls_builtin_without_fhls : Error <
  "HLS builtin can only be used when the HLS flag is present on the command "
  "line invocation">;
def err_ap_int_type : Error <
  "invalid __ap_int element type %0; must be 'int' or 'unsigned int'">;
def err_ap_int_bad_size : Error <
  "%select{signed|unsigned|}0 __ap_int must have a size of at least %select{2|1|1}0">;
def err_fpga_reg_limitations : Error <
  "Illegal %select{argument of type %1 |field in argument}0 to __builtin_fpga_reg.">;
def err_pragma_loop_invalid_negative_value : Error<
  "invalid value '%0'; must be non-negative">;

// IntrinsicPromotion implementation.
def warn_intrinsic_promote : Warning<"Function target features %0 added due to"
  " call to an intrinsic">,
InGroup<DiagGroup<"intrinsic-promote">>;
def note_intrinsic_promote : Note<"Intrinsic called here">;

// OpenMP
def err_omp_single_decl_in_declare_variant : Error<
  "single declaration is expected after 'declare variant' directive">;
def err_omp_function_expected_variant : Error<
  "'#pragma omp declare variant' can only be applied to functions">;
def warn_omp_simdlen_in_target_spir: Warning<
  "simdlen clause is not supported for this target">,
  InGroup<SimdUnsupported>;

#if INTEL_FEATURE_CSA
// Intel CSA extensions
def err_csa_builtin_arg_mismatch : Error<
  "CSA builtin parameter must be %select{"
  "a scalar or a vector type|an integer|a pointer}0">;
#endif // INTEL_FEATURE_CSA

} // end of sema component.
