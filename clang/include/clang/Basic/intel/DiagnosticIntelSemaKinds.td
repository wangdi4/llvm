
//==--- DiagnosticSemaKinds.td - libsema diagnostics ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Semantic Analysis
//===----------------------------------------------------------------------===//

let Component = "Sema" in {
let CategoryName = "Semantic Issue" in {
// if INTEL_CUSTOMIZATION
// CQ#364268
def warn_main_arg_wrong : Warning<"%select{first|second|third|fourth}0 "
    "parameter of 'main' (%select{argument count|argument array|environment|"
    "platform-specific data}0) should be of type %1">, InGroup<Main>;
// CQ#366562 - allow #pragma unroll with non-positive or too large argument.
def warn_pragma_unroll_invalid_factor_ignored : Warning<
  "unrolling factor '%0' is %select{negative|too large}1 and will be ignored">,
  InGroup<Pragmas>;
// Fix for CQ373961: redeclaration of class member with different access.
def warn_class_redeclared_with_different_access : Warning<
  "%0 redeclared with '%1' access">,
  AccessControl, InGroup<RedeclaredClassMember>;
def warn_access_dtor_vbase :
    Warning<"inherited virtual base class %1 has "
    "%select{private|protected}2 destructor">,
    AccessControl, InGroup<PrivateVBaseDtor>;
def warn_intel_nontrivial_union_or_anon_struct_member : Warning<
  "%select{anonymous struct|union}0 member %1 has a non-trivial "
  "%select{constructor|copy constructor|move constructor|copy assignment "
  "operator|move assignment operator|destructor}2">,
  InGroup<IntelCompat>;
def warn_lvalue_reference_bind_to_initlist : Warning<
  "non-const lvalue reference to type %0 cannot bind to an "
  "initializer list temporary">, InGroup<IntelCompat>;
def warn_lvalue_reference_bind_to_temporary : Warning<
  "non-const lvalue reference %diff{to type $ cannot bind "
  "to a temporary of type $|cannot bind to incompatible temporary}0,1">,
  InGroup<IntelCompat>;
// CQ#236476 - static variable is referenced in two separate routines in iclang.
def err_static_variable_with_label_addr
    : Error<"cannot initialize static variable with label address">;
// CQ#375011
def err_alloc_size_attribute_argument_not_int : Error<
  "'alloc_size' attribute requires integer constant">;
def warn_attribute_requires_integer_param_type : Warning<
  "%0 attribute requires an integer function parameter">,
  InGroup<IgnoredAttributes>;
def note_param_index_specified_here : Note<
  "parameter's index specified here">;
// CQ#370092
def warn_decl_attribute_invalid_on_stmt : Warning<
  "%0 attribute cannot be applied to a statement; "
  "attribute ignored">,
  InGroup<IgnoredAttributes>;
// CQ#370092
def warn_mismatched_visibility: Warning<"visibility does not match previous declaration; "
  "attribute ignored">,
  InGroup<IgnoredAttributes>;
// CQ#376510: Same as err_ident_list_in_fn_declaration, but a warning.
def warn_ident_list_in_fn_declaration: Warning<
  err_ident_list_in_fn_declaration.Text>,
  InGroup<IntelCompat>;
def warn_default_arg_makes_ctor_special : Warning<
  "addition of default argument on redeclaration makes this constructor a "
  "%select{default|copy|move}0 constructor">, InGroup<IntelCompat>;
// endif INTEL_CUSTOMIZATION

// if INTEL_CUSTOMIZATION
// CQ#373258 - allow class member typedef redefinition in IntelCompat mode.
def ext_intel_member_typedef_redefinition : ExtWarn<
  "class member typedef redefinition is an Intel extension">,
  InGroup<IntelCompat>;
// CQ#377372 - allow to redefine extern inline functions in C++ mode
def ext_intel_redefinition_extern_inline : ExtWarn<
  "redefinition of a 'extern inline' function %0 in "
  "C++ mode is an Intel extension">,
  InGroup<IntelCompat>;
// CQ#369830 - static declaration of functions is treated differently.
def ext_intel_static_non_static : Extension<
  "redeclaring non-static %0 as static is an Intel extension">,
  InGroup<IntelCompat>;
def ext_intel_non_static_static : Extension<
  "redeclaring static %0 as non-static is an Intel extension">,
  InGroup<IntelCompat>;
def ext_intel_static_block_func : Extension<
  "static functions inside blocks are Intel extension">, InGroup<IntelCompat>;
// CQ#364598, CQ#374878.
def ext_intel_elaborated_type_refers_to_typedef : ExtWarn<
  "elaborated type refers to a typedef">, InGroup<IntelCompat>;
// CQ#366612
def warn_func_redecl_conflicting_types: Warning<
  "declaration of %0 is incompatible with previous "
  "%select{declaration|definition}1">, InGroup<IntelCompat>;
// CQ#365886
def ext_intel_forward_ref_enum : ExtWarn<
  "forward references to 'enum' types are an Intel extension">,
  InGroup<IntelCompat>;
// CQ370802 - allows bypassing var init in switch statement if IntelCompat
def warn_switch_into_protected_scope : Warning<
  "jump from switch statement to this case label bypasses variable initialization">,
  InGroup<IntelCompat>;
// CQ370802 - allows indirect goto to const address if IntelCompat
def warn_indirect_goto_with_const_expr : Warning<
  "expr in 'goto *expr' is constant">, InGroup<IntelCompat>;
// CQ370802 - allows indirect got to any places
def warn_indirect_goto_in_protected_scope : Warning<
  err_indirect_goto_in_protected_scope.Text>, InGroup<IntelCompat>;
  // CQ#366309 - allow arrays with incomplete element type in IntelCompat mode.
def ext_intel_array_incomplete_type : Extension<
  "array with incomplete element type %0 is an Intel extension">,
  InGroup<IntelCompat>;
  // CQ#380872 - Arrays with incomplete (unknown) size
def warn_intel_array_incomplete_size : Warning<
  "array with incomplete (unknown) size element %0">,
  InGroup<IntelCompat>;
// Fix for CQ375472.
def warn_conditional_ambiguous : Warning<
  "conditional expression is ambiguous; "
  "%diff{$ can be converted to $ and vice versa|"
  "types can be convert to each other}0,1">, InGroup<IntelCompat>;
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
// Same as ext_return_has_expr, but a warning, not an error, for
// IntelCompat mode. CQ#367767
def warn_ext_return_has_expr : Warning<
  "%select{void function|void method|constructor|destructor}1 %0 "
  "should not return a value">, InGroup<IntelCompat>;
// CQ#371990 - let argument be out of Low..High range, assuming Low.
def warn_arg_invalid_range_low_assumed : Warning<
  "expected a value between %0 and %1, defaults to %0">,
  InGroup<IntelCompat>;
// CQ#371340
def warn_non_asm_stmt_in_naked_function : Warning<
  "attribute 'naked' ignored on function with non-ASM statements">,
  InGroup<IgnoredAttributes>;
// endif INTEL_CUSTOMIZATION

// if INTEL_SPECIFIC_CILKPLUS
def warn_empty_cilk_for_body : Warning<
  "Cilk for loop has empty body">, InGroup<EmptyBody>;
def warn_empty_simd_for_body : Warning<
  "SIMD for loop has empty body">, InGroup<EmptyBody>;
def err_pragma_simd_reuse_var : Error<
  "%select{private|lastprivate|firstprivate|linear|reduction}0 variable shall "
  "not appear in multiple simd clauses">;
def err_pragma_simd_reduction_maxmin : Error<
  "reduction operator %select{min|max}0 requires arithmetic type">;
def note_pragma_simd_used_here : Note<
  "first used here">;
def err_pragma_simd_conflict_step : Error<
  "linear %select{variable|step}0 shall not be used"
  " as a linear %select{step|variable}0">;
def err_pragma_simd_vectorlength_conflict : Error<
  "cannot specify both vectorlength%select{for|}0 and vectorlength%select{|for}0">;
def err_pragma_simd_vectorlength_multiple : Error<
  "cannot specify multiple vectorlength%select{for|}0 clauses">;
def note_pragma_simd_specified_here : Note<
  "vectorlength%select{for|}0 first specified here">;
def err_invalid_vectorlength_expr : Error<
  "vectorlength expression must be %select{an integer constant|a power of 2}0">;
def err_pragma_simd_invalid_linear_step : Error<
  "invalid linear step: expected integral constant or variable reference">;
def warn_pragma_simd_linear_expr_zero : Warning<
  "linear step is zero">, InGroup<SourceUsesCilkPlus>, DefaultIgnore;
def err_pragma_simd_invalid_private_var : Error<
  "invalid variable in %0 clause">;
def err_pragma_simd_var_incomplete : Error<
  "variable in %0 clause shall not be incomplete: %1">;
def err_pragma_simd_var_reference : Error<
  "variable in %0 clause shall not be a reference: %1">;
def err_pragma_simd_var_const: Error<
  "variable in %0 clause shall not be const-qualified">;
def err_pragma_simd_var_array: Error<
  "variable in %0 clause shall not be of array type">;
def note_pragma_simd_var: Note<
  "variable declared here">;
def err_pragma_simd_var_no_member: Error<
  "variable in %0 clause has no "
  "%select{default constructor|default copy constructor|copy assignment operator}1 "
  "or it is deleted: %2">;
def err_pragma_simd_var_ambiguous_member: Error<
  "variable in %0 clause has ambiguous "
  "%select{default constructors|default copy constructors|copy assignment operators}1: %2">;
def err_pragma_simd_reduction_invalid_var : Error<
  "invalid reduction variable">;

// Cilk Plus warnings and errors
// cilk_spawn
def err_not_a_call : Error<
  "the argument to _Cilk_spawn must be a function call">;
def err_spawn_invalid_scope : Error<
  "_Cilk_spawn not allowed in this scope">;
def err_spawn_invalid_decl : Error<
  "_Cilk_spawn not supported in a '%0Decl'">;
def err_spawn_spawn : Error<
  "consecutive _Cilk_spawn tokens not allowed">;
def err_spawn_not_whole_expr : Error<
  "_Cilk_spawn is not at statement level">;
def err_cannot_spawn_function: Error<
  "function cannot be spawned">;
def err_cannot_init_static_variable: Error<
  "cannot spawn initialize a 'static' variable">;
def err_multiple_spawns: Error<
  "multiple spawn calls within a full expression">;
def note_multiple_spawns: Note<
  "another spawn here">;
def err_cilk_spawn_disable: Error<
  "_Cilk_spawn disabled in statement expression">;

// cilk_for
def err_cilk_for_initializer_expected_variable : Error<
  "expected a variable for control variable in '_Cilk_for'">;
def err_cilk_for_initializer_expected_decl : Error<
  "expected control variable declaration in initializer in '_Cilk_for'">;
def err_cilk_for_decl_multiple_variables : Error<
  "cannot declare more than one loop control variable in '_Cilk_for'">;
def err_cilk_for_init_multiple_variables : Error<
  "cannot initialize more than one loop control variable in '_Cilk_for'">;
def err_cilk_for_control_variable_not_initialized : Error<
  "_Cilk_for loop control variable must be initialized">;
def err_cilk_for_initialization_must_be_decl : Error<
  "loop initialization must be a declaration in '_Cilk_for'">;

def err_cilk_for_control_variable_storage_class : Error<
  "loop control variable cannot have storage class '%0' in '_Cilk_for'">;
def err_cilk_for_control_variable_qualifier : Error<
  "loop control variable cannot be '%0' in '_Cilk_for'">;
def err_cilk_for_control_variable_not_local : Error<
  "non-local loop control variable in '_Cilk_for'">;
def err_cilk_for_control_variable_type : Error<
  "loop control variable must have an integral, pointer, or class type "
  "in '_Cilk_for'">;

def err_cilk_for_invalid_cond_expr: Error<
  "expected binary comparison operator in '_Cilk_for' loop condition">;
def err_cilk_for_invalid_cond_operator: Error<
  "loop condition operator must be one of '<', '<=', '>', '>=', or '!=' "
  "in '_Cilk_for'">;
def err_cilk_for_cond_test_control_var: Error<
  "loop condition does not test control variable %0 in '_Cilk_for'">;
def note_cilk_for_cond_allowed: Note<
  "allowed forms are %0 OP expr, and expr OP %0">;
def err_cilk_for_difference_ill_formed: Error<
  "end - begin must be well-formed in '_Cilk_for'">;
def err_non_integral_cilk_for_difference_type: Error<
  "end - begin must have integral type in '_Cilk_for' - got %0">;

def err_cilk_for_increment_not_control_var: Error<
  "loop increment does not modify control variable %0 in '_Cilk_for'">;
def err_cilk_for_invalid_increment : Error<
  "loop increment operator must be one of operators '++', '--', '+=', or '-=' in '_Cilk_for'">;
def err_cilk_for_invalid_increment_rhs : Error<
  "right-hand side of '%0' must have integral or enum type in '_Cilk_for' increment">;
def err_cilk_for_increment_inconsistent : Error<
  "loop increment is inconsistent with condition in '_Cilk_for': expected %select{negative|positive}0 stride">;
def err_cilk_for_increment_zero : Error<
  "loop increment must be non-zero in '_Cilk_for'">;
def err_cilk_for_loop_modifies_control_var : Error<
  "Modifying the loop control variable inside a '_Cilk_for' has undefined behavior">;
def warn_cilk_for_loop_control_var_func : Warning<
  "Modifying the loop control variable inside a '_Cilk_for' using a function call has undefined behavior">, InGroup<CilkPlusLoopControlVarModification>, DefaultIgnore;
def warn_cilk_for_loop_control_var_aliased: Warning<
  "Modifying the loop control variable '%0' through an alias in '_Cilk_for' has undefined behavior">, InGroup<CilkPlusLoopControlVarModification>, DefaultIgnore;
def note_cilk_for_loop_control_var_declared_here: Note<
  "'_Cilk_for' loop control variable declared here">;

def note_cilk_for_begin_expr: Note<
  "loop begin expression here">;
def note_cilk_for_end_expr: Note<
  "loop end expression here">;
def note_constant_stride: Note<
  "constant stride is %0">;
def warn_cilk_for_cond_user_defined_conv: Warning<
  "user-defined conversion from %0 to %1 will not be used when calculating the "
  "number of iterations in '_Cilk_for'">, InGroup<Conversion>, DefaultWarn;
def note_cilk_for_conversion_here : Note<
  "conversion to type %0 declared here">;
def err_cilk_for_cannot_return: Error<
  "cannot return from within a '_Cilk_for' loop">;
def err_cilk_for_cannot_break: Error<
  "cannot break from a '_Cilk_for' loop">;
def warn_cilk_for_loop_count_downcast: Warning<
  "implicit loop count downcast from %0 to %1 in '_Cilk_for'">,
  InGroup<Conversion>, DefaultWarn;
def note_local_variable_declared_here : Note<
  "%0 declared here">;

def err_cilk_for_grainsize_negative: Error<
  "the behavior of Cilk for is unspecified for a negative grainsize">;
def note_cilk_for_grainsize_conversion : Note<
  "grainsize must evaluate to a type convertible to %0">;

def warn_cilk_for_wraparound: Warning<
  "%0 stride causes %1 wraparound">, InGroup<SourceUsesCilkPlus>, DefaultWarn;

def note_cilk_for_wraparound_undefined: Note<
  "wraparounds cause undefined behavior in Cilk for">;

def err_cilk_elemental_not_function_parameter: Error<
  "not a function parameter">;
def err_cilk_elemental_vectorlength: Error<
  "vectorlength must be positive">;
def err_cilk_elemental_unrecognized_processor: Error<
  "unrecognized processor">;
def err_cilk_elemental_linear_parameter_type: Error<
  "linear parameter type %0 is not an integer or pointer type">;
def warn_cilk_elemental_inconsistent_processor: Warning<
  "inconsistent processor attribute">,
  InGroup<SourceUsesCilkPlus>, DefaultWarn;
def err_cilk_elemental_exception_spec : Error<
  "exception specifications are not allowed on elemental functions">;
def err_cilk_elemental_step_not_uniform : Error<
  "linear step parameter must also be uniform">;
def err_cilk_elemental_subject: Error<
  "parameter %0 cannot be the subject of two elemental clauses">;
def err_cilk_elemental_this_subject: Error<
  "'this' cannot be the subject of two elemental clauses">;
def note_cilk_elemental_subject_clause: Note<
  "another %select{uniform|linear}0 clause here">;
def note_cilk_elemental_subject_parameter: Note<
  "parameter here">;
def err_cilk_elemental_linear_step_not_integral : Error<
  "linear step has non-integer type %0">;
def err_cilk_elemental_linear_step_not_constant : Error<
  "linear step is not constant">;
def err_cilk_elemental_aligned_not_integral : Error<
  "aligned has non-integer type %0">;
def err_cilk_elemental_aligned_not_constant : Error<
  "aligned is not a constant">;
def err_cilk_elemental_repeated_vectorlength: Error<
  "repeated %select{vectorlength|vectorlengthfor}0 attribute is not allowed">;
def err_cilk_elemental_both_mask_nomask : Error<
  "elemental function cannot have both mask and nomask attributes">;
def note_cilk_elemental_mask_here: Note<
  "%select{nomask|mask}0 attribute here">;

// simd for
def err_simd_for_body_no_construct: Error<
  "%0 is not allowed within %select{simd for|an elemental function}1">;
def err_simd_for_nested: Error<
  "nested simd for not supported">;
def note_simd_for_nested: Note<
  "outer simd for here">;
def err_simd_for_body_no_nontrivial_destructor: Error<
  "non-static variable with a non-trivial destructor is not allowed within %select{simd for|an elemental function}0">;
def err_simd_for_decl_multiple_variables : Error<
  "cannot declare more than one loop control variable in simd for">;
def err_simd_for_initializer_expected_decl : Error<
  "expected control variable declaration in simd for initialization">;
def err_simd_for_invalid_lcv_type : Error<
  "loop control variable shall have an integer or pointer type, %0 type here">;
def err_simd_for_control_variable_not_initialized : Error<
  "simd for loop control variable must be initialized">;
def err_simd_for_initialization_must_be_decl : Error<
  "loop initialization must be a declaration in simd for">;
def err_simd_for_init_multiple_variables : Error<
  "cannot initialize more than one loop control variable in simd for">;
def err_simd_for_expect_loop_control_variable : Error<
  "expect a loop control variable in simd for">;
def err_simd_for_control_variable_storage_class : Error<
  "loop control variable cannot have storage class '%0' in simd for">;
def err_simd_for_control_variable_qualifier : Error<
  "loop control variable cannot be '%0' in simd for">;
def err_simd_for_control_variable_not_local : Error<
  "non-local loop control variable in simd for">;
def err_simd_for_invalid_cond_expr: Error<
  "expected binary comparison operator in simd for loop condition">;
def err_simd_for_invalid_cond_operator: Error<
  "loop condition operator must be one of '<', '<=', '>', '>=', or '!=' "
  "in simd for">;
def err_simd_for_cond_test_control_var: Error<
  "loop condition does not test control variable %0 in simd for">;
def note_simd_for_cond_allowed: Note<
  "allowed forms are %0 OP expr, and expr OP %0">;
def err_simd_for_increment_not_control_var: Error<
  "loop increment does not modify control variable %0 in simd for">;
def err_simd_for_invalid_increment : Error<
  "loop increment operator must be one of operators '++', '--', '+=', or '-=' in simd for">;
def err_simd_for_invalid_increment_rhs : Error<
  "right-hand side of '%0' must have integral or enum type in simd for increment">;
def err_simd_for_increment_zero : Error<
  "loop increment must be non-zero in simd for">;
def err_simd_for_increment_inconsistent : Error<
  "loop increment is inconsistent with condition in simd for: expected %select{negative|positive}0 stride">;
def err_simd_for_cannot_return: Error<
  "cannot return from within a simd for loop">;
def err_simd_for_cannot_break: Error<
  "cannot break from a simd for loop">;
def err_simd_for_lcv_invalid_clause: Error<
  "the simd loop control variable may not be the subject of a %select{reduction|firstprivate}0 clause">;
def warn_simd_for_variable_lcv: Warning<
  "ignoring simd clause applied to simd loop control variable">,
  InGroup<SourceUsesCilkPlus>, DefaultWarn;

def err_cean_incorrect_form : Error<
  "incorrect form of extended array notation">;
def err_cean_no_length_for_non_array : Error<
  "cannot define default length for non-array type %0">;
def err_cean_lower_bound_not_integer : Error<
  "lower bound expression has non-integer type %0">;
def err_cean_length_not_integer : Error<
  "length expression has non-integer type %0">;
def err_cean_stride_not_integer : Error<
  "stride expression has non-integer type %0">;
def err_cean_not_in_statement : Error<
  "extended array notation is not allowed">;
def warn_cean_wrong_length : Warning<
  "length is %select{negative|zero}0">, InGroup<CilkPlusCEAN>, DefaultWarn;
def err_cean_rank_mismatch : Error<
  "rank mismatch in array section expression">;
def note_cean_if_rank : Note<
  "rank is defined by condition">;
def err_cean_different_length : Error<
  "different length value in extended array notation expression">;
def note_cean_different_length : Note<
  "length specified here">;
def err_cean_rank_zero : Error<
  "argument of the function is not an array section">;
def err_cean_rank_not_one : Error<
  "rank of the argument must be exactly one">;
def err_cean_rank_not_zero : Error<
  "rank of the argument must be zero">;
def err_cean_expr_rank_not_zero : Error<
  "rank of the expression must be zero">;
def err_cean_rank_not_zero_or_one : Error<
  "rank of the argument must be not greater than one">;
def err_cean_not_scalar_or_complex : Error<
  "type of argument is not scalar or complex">;
def err_cean_not_scalar : Error<
  "type of argument is not scalar">;
def err_cean_not_integer : Error<
  "type of argument is not integer">;
def err_cean_less_zero : Error<
  "value of argument is negative integer">;
def err_cean_arg_not_lvalue : Error<
  "argument expression must be an l-value">;
// endif INTEL_SPECIFIC_CILKPLUS

// if INTEL_CUSTOMIZATION
// CQ#373129 - support for __assume_aligned builtin.
def err_assume_aligned_first_arg_not_pointer : Error<
  "first argument to '__assume_aligned' must be a pointer">;
def warn_bad_cxx_cast_generic : Warning<
  "%select{const_cast|static_cast|reinterpret_cast|dynamic_cast|C-style cast|"
  "functional-style cast}0 from %1 to %2 is allowed in Intel/GCC compatibility mode only">,
  InGroup<IntelCompat>;
def warn_bad_cxx_cast_unrelated_class : Warning<
  "%select{const_cast|static_cast|reinterpret_cast|dynamic_cast|C-style cast|"
  "functional-style cast}0 from %1 to %2, which are not related by "
  "inheritance, is allowed in Intel/GCC compatibility mode only">, InGroup<IntelCompat>;
// endif INTEL_CUSTOMIZATION
}
// if INTEL_SPECIFIC_IL0_BACKEND
let CategoryName = "IL0 Issue" in {
def x_warn_intel_pragma_opt_level : Warning<
  "optimization level must be between 0 and 3">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_function_only : Warning<
  "optimization level applies only to function definitions">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_invalid_expr : Warning<
  "invalid expression in pragma will be ignored">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_use_undecl : Warning<
  "name of variable is expected">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_var_local : Warning<
  "variable %0 has local storage">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_not_C : Warning<
  "%select{variable|function}0 %1 has no \"C\" linkage specification">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_identifier_undefined : Warning<
  "identifier %0 is undefined">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_func_alloc_text : Warning<
  "text segment already specified">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_init_seg_defined : Warning<
  "initialization segment already defined">,
  InGroup<IntelPragma>;
def x_warn_intel_pragma_ignore_attributes_standard_section : Warning<"attributes for standard section %0 ignored">,
  InGroup<IntelPragma>;
def x_warn_intel_attribute_variable_only : Warning<"attribute %0 can be applied only to non-reference variables">,
  InGroup<IntelPragma>;
def x_error_intel_pragma_except_off : Error<"exception semantics must be turned off before turning off this float_control mode">;
def x_error_intel_pragma_fenv_access_off : Error<"fenv_access must be turned off before turning off this float_control mode">;
def x_error_intel_pragma_fenv_must_be_precise : Error<"fenv_access cannot be enabled except in precise, source, double and extended modes">;
def x_error_intel_pragma_must_be_precise : Error<"exception semantics cannot be enabled except in precise, source, double and extended modes">;
def x_error_intel_pragma_parallel_pointer_array_is_allowed : Error<"expecting an array or pointer variable">;
def x_error_intel_pragma_parallel_subscript_out_of_range : Error<"subscript out of range">;
def x_error_intel_pragma_conflicting_section_attributes : Error<"conflicting section attributes">;
def x_err_attribute_not_string : Error<"argument to %0 attribute was not a string literal">;
def err_attribute_section_local_variable : Error<
  "'section' attribute is not valid on local variables">;
}
// endif INTEL_SPECIFIC_IL0_BACKEND
} // end of sema component.
