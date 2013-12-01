// ****************************************************************************
//     Snippets of code which constitute implementations of Work-group BI
//     functions in OpenCL. They should be defined as class members, and
//     thus must reside on the top-level of TableGen hierarchy
//     (due to limitations of Volcano TableGen back-end design which
//     places all definitions inside an encompassing class)
// ****************************************************************************

class WorkGroupSnippet<code Impl> {
  code snippet = Impl;
}

// -------------------------------------------
// All, Any, Reduce and Scan: function bodies
// -------------------------------------------

def work_group_scan_inclusive_function_op_vector4 : WorkGroupSnippet<[{
  $Arg0VecType ret;
  $Arg0VecType acc = *$Arg1VarName;
  ret.s0 = function_op($Arg0VarName.s0, acc.s3);
  ret.s1 = function_op($Arg0VarName.s1, ret.s0);
  ret.s2 = function_op($Arg0VarName.s2, ret.s1);
  ret.s3 = function_op($Arg0VarName.s3, ret.s2);
  *$Arg1VarName = ret;
  return ret;
}]>;

def work_group_scan_inclusive_function_op_vector8 : WorkGroupSnippet<[{
  $Arg0VecType ret;
  $Arg0VecType acc = *$Arg1VarName;
  ret.s0 = function_op($Arg0VarName.s0, acc.s7);
  ret.s1 = function_op($Arg0VarName.s1, ret.s0);
  ret.s2 = function_op($Arg0VarName.s2, ret.s1);
  ret.s3 = function_op($Arg0VarName.s3, ret.s2);
  ret.s4 = function_op($Arg0VarName.s4, ret.s3);
  ret.s5 = function_op($Arg0VarName.s5, ret.s4);
  ret.s6 = function_op($Arg0VarName.s6, ret.s5);
  ret.s7 = function_op($Arg0VarName.s7, ret.s6);
  *$Arg1VarName = ret;
  return ret;
}]>;

def work_group_scan_inclusive_function_op_vector16 : WorkGroupSnippet<[{
  $Arg0VecType ret;
  $Arg0VecType acc = *$Arg1VarName;
  ret.s0 = function_op($Arg0VarName.s0, acc.sF);
  ret.s1 = function_op($Arg0VarName.s1, ret.s0);
  ret.s2 = function_op($Arg0VarName.s2, ret.s1);
  ret.s3 = function_op($Arg0VarName.s3, ret.s2);
  ret.s4 = function_op($Arg0VarName.s4, ret.s3);
  ret.s5 = function_op($Arg0VarName.s5, ret.s4);
  ret.s6 = function_op($Arg0VarName.s6, ret.s5);
  ret.s7 = function_op($Arg0VarName.s7, ret.s6);
  ret.s8 = function_op($Arg0VarName.s8, ret.s7);
  ret.s9 = function_op($Arg0VarName.s9, ret.s8);
  ret.sA = function_op($Arg0VarName.sA, ret.s9);
  ret.sB = function_op($Arg0VarName.sB, ret.sA);
  ret.sC = function_op($Arg0VarName.sC, ret.sB);
  ret.sD = function_op($Arg0VarName.sD, ret.sC);
  ret.sE = function_op($Arg0VarName.sE, ret.sD);
  ret.sF = function_op($Arg0VarName.sF, ret.sE);
  *$Arg1VarName = ret;
  return ret;
}]>;

def work_group_scan_exclusive_function_op_vector4 : WorkGroupSnippet<[{
  $Arg0VecType ret;
  $Arg0VecType acc = *$Arg1VarName;
  ret.s0 = acc.s3;
  ret.s1 = acc.s0 = function_op($Arg0VarName.s0, ret.s0);
  ret.s2 = acc.s1 = function_op($Arg0VarName.s1, ret.s1);
  ret.s3 = acc.s2 = function_op($Arg0VarName.s2, ret.s2);
  acc.s3 = function_op($Arg0VarName.s3, ret.s3);
  *$Arg1VarName = acc;
  return ret;
}]>;

def work_group_scan_exclusive_function_op_vector8 : WorkGroupSnippet<[{
  $Arg0VecType ret;
  $Arg0VecType acc = *$Arg1VarName;
  ret.s0 = acc.s7;
  ret.s1 = acc.s0 = function_op($Arg0VarName.s0, ret.s0);
  ret.s2 = acc.s1 = function_op($Arg0VarName.s1, ret.s1);
  ret.s3 = acc.s2 = function_op($Arg0VarName.s2, ret.s2);
  ret.s4 = acc.s3 = function_op($Arg0VarName.s3, ret.s3);
  ret.s5 = acc.s4 = function_op($Arg0VarName.s4, ret.s4);
  ret.s6 = acc.s5 = function_op($Arg0VarName.s5, ret.s5);
  ret.s7 = acc.s6 = function_op($Arg0VarName.s6, ret.s6);
  acc.s7 = function_op($Arg0VarName.s7, ret.s7);
  *$Arg1VarName = acc;
  return ret;
}]>;

def work_group_scan_exclusive_function_op_vector16 : WorkGroupSnippet<[{
  $Arg0VecType ret;
  $Arg0VecType acc = *$Arg1VarName;
  ret.s0 = acc.sF;
  ret.s1 = acc.s0 = function_op($Arg0VarName.s0, ret.s0);
  ret.s2 = acc.s1 = function_op($Arg0VarName.s1, ret.s1);
  ret.s3 = acc.s2 = function_op($Arg0VarName.s2, ret.s2);
  ret.s4 = acc.s3 = function_op($Arg0VarName.s3, ret.s3);
  ret.s5 = acc.s4 = function_op($Arg0VarName.s4, ret.s4);
  ret.s6 = acc.s5 = function_op($Arg0VarName.s5, ret.s5);
  ret.s7 = acc.s6 = function_op($Arg0VarName.s6, ret.s6);
  ret.s8 = acc.s7 = function_op($Arg0VarName.s7, ret.s7);
  ret.s9 = acc.s8 = function_op($Arg0VarName.s8, ret.s8);
  ret.sA = acc.s9 = function_op($Arg0VarName.s9, ret.s9);
  ret.sB = acc.sA = function_op($Arg0VarName.sA, ret.sA);
  ret.sC = acc.sB = function_op($Arg0VarName.sB, ret.sB);
  ret.sD = acc.sC = function_op($Arg0VarName.sC, ret.sC);
  ret.sE = acc.sD = function_op($Arg0VarName.sD, ret.sD);
  ret.sF = acc.sE = function_op($Arg0VarName.sE, ret.sE);
  acc.sF = function_op($Arg0VarName.sF, ret.sF);
  *$Arg1VarName = acc;
  return ret;
}]>;

def work_group_inclusive_function_op_vector : WorkGroupSnippet<[{
  $Arg0VecType acc = function_op($Arg0VarName, *$Arg1VarName);
  *$Arg1VarName = acc;
  return acc;
}]>;

def work_group_inclusive_function_op_scalar : WorkGroupSnippet<[{
  $Arg0Type acc = function_op($Arg0VarName, *$Arg1VarName);
  *$Arg1VarName = acc;
  return acc;
}]>;

def work_group_scan_exclusive_function_op_scalar : WorkGroupSnippet<[{
  $Arg0Type acc = *$Arg1VarName;
  *$Arg1VarName = function_op($Arg0VarName, *$Arg1VarName);
  return acc;
}]>;

// -----------------------------
// Finalization function bodies
// -----------------------------

def work_group_finalize_function_op4 : WorkGroupSnippet<[{
  $Arg0VecType ret;
  ret.s0 = $Arg0VarName.s0;
  ret.s0 = function_op($Arg0VarName.s1, ret.s0);
  ret.s0 = function_op($Arg0VarName.s2, ret.s0);
  ret.s0 = function_op($Arg0VarName.s3, ret.s0);
  return ($Arg0VecType)(ret.s0);
}]>;

def work_group_finalize_function_op8 : WorkGroupSnippet<[{
  $Arg0VecType ret;
  ret.s0 = $Arg0VarName.s0;
  ret.s0 = function_op($Arg0VarName.s1, ret.s0);
  ret.s0 = function_op($Arg0VarName.s2, ret.s0);
  ret.s0 = function_op($Arg0VarName.s3, ret.s0);
  ret.s0 = function_op($Arg0VarName.s4, ret.s0);
  ret.s0 = function_op($Arg0VarName.s5, ret.s0);
  ret.s0 = function_op($Arg0VarName.s6, ret.s0);
  ret.s0 = function_op($Arg0VarName.s7, ret.s0);
  return ($Arg0VecType)(ret.s0);
}]>;

def work_group_finalize_function_op16 : WorkGroupSnippet<[{
  $Arg0VecType ret;
  ret.s0 = $Arg0VarName.s0;
  ret.s0 = function_op($Arg0VarName.s1,  ret.s0);
  ret.s0 = function_op($Arg0VarName.s2,  ret.s0);
  ret.s0 = function_op($Arg0VarName.s3,  ret.s0);
  ret.s0 = function_op($Arg0VarName.s4,  ret.s0);
  ret.s0 = function_op($Arg0VarName.s5,  ret.s0);
  ret.s0 = function_op($Arg0VarName.s6,  ret.s0);
  ret.s0 = function_op($Arg0VarName.s7,  ret.s0);
  ret.s0 = function_op($Arg0VarName.s8,  ret.s0);
  ret.s0 = function_op($Arg0VarName.s9,  ret.s0);
  ret.s0 = function_op($Arg0VarName.sA,  ret.s0);
  ret.s0 = function_op($Arg0VarName.sB,  ret.s0);
  ret.s0 = function_op($Arg0VarName.sC,  ret.s0);
  ret.s0 = function_op($Arg0VarName.sD,  ret.s0);
  ret.s0 = function_op($Arg0VarName.sE,  ret.s0);
  ret.s0 = function_op($Arg0VarName.sF,  ret.s0);
  return ($Arg0VecType)(ret.s0);
}]>;

// ---------------------------------
// Classes with specialized snippets
// ---------------------------------
class WgSnippetSpecialized<string op, code template> {
  string specialized = !subst("function_op", op, template);
}

// work_group_all
def snippet_work_group_all_vector_helper_int : WgSnippetSpecialized<"work_group_all_util", work_group_inclusive_function_op_vector.snippet>;
def snippet_work_group_all_scalar_helper_int : WgSnippetSpecialized<"work_group_all_util", work_group_inclusive_function_op_scalar.snippet>;

// work_group_any
def snippet_work_group_any_vector_helper_int : WgSnippetSpecialized<"work_group_any_util", work_group_inclusive_function_op_vector.snippet>;
def snippet_work_group_any_scalar_helper_int : WgSnippetSpecialized<"work_group_any_util", work_group_inclusive_function_op_scalar.snippet>;

// work_group_reduce_add
def snippet_work_group_reduce_add_vector_helper_generic : WgSnippetSpecialized<"work_group_add_util", work_group_inclusive_function_op_vector.snippet>;
def snippet_work_group_reduce_add_scalar_helper_generic : WgSnippetSpecialized<"work_group_add_util", work_group_inclusive_function_op_scalar.snippet>;

// work_group_reduce_max/min
def snippet_work_group_reduce_min_vector_helper_int   : WgSnippetSpecialized<"min",  work_group_inclusive_function_op_vector.snippet>;
def snippet_work_group_reduce_min_scalar_helper_int   : WgSnippetSpecialized<"min",  work_group_inclusive_function_op_scalar.snippet>;
def snippet_work_group_reduce_min_vector_helper_float : WgSnippetSpecialized<"fmin", work_group_inclusive_function_op_vector.snippet>;
def snippet_work_group_reduce_min_scalar_helper_float : WgSnippetSpecialized<"fmin", work_group_inclusive_function_op_scalar.snippet>;
def snippet_work_group_reduce_max_vector_helper_int   : WgSnippetSpecialized<"max",  work_group_inclusive_function_op_vector.snippet>;
def snippet_work_group_reduce_max_scalar_helper_int   : WgSnippetSpecialized<"max",  work_group_inclusive_function_op_scalar.snippet>;
def snippet_work_group_reduce_max_vector_helper_float : WgSnippetSpecialized<"fmax", work_group_inclusive_function_op_vector.snippet>;
def snippet_work_group_reduce_max_scalar_helper_float : WgSnippetSpecialized<"fmax", work_group_inclusive_function_op_scalar.snippet>;

// vector versions of Scan_add/min/max functions
def snippet_work_group_scan_exclusive_add_vector_helper_gen4    : WgSnippetSpecialized<"work_group_add_util", work_group_scan_exclusive_function_op_vector4.snippet>;
def snippet_work_group_scan_exclusive_add_vector_helper_gen8    : WgSnippetSpecialized<"work_group_add_util", work_group_scan_exclusive_function_op_vector8.snippet>;
def snippet_work_group_scan_exclusive_add_vector_helper_gen16   : WgSnippetSpecialized<"work_group_add_util", work_group_scan_exclusive_function_op_vector16.snippet>;
def snippet_work_group_scan_exclusive_min_vector_helper_int4    : WgSnippetSpecialized<"min",  work_group_scan_exclusive_function_op_vector4.snippet>;
def snippet_work_group_scan_exclusive_min_vector_helper_int8    : WgSnippetSpecialized<"min",  work_group_scan_exclusive_function_op_vector8.snippet>;
def snippet_work_group_scan_exclusive_min_vector_helper_int16   : WgSnippetSpecialized<"min",  work_group_scan_exclusive_function_op_vector16.snippet>;
def snippet_work_group_scan_exclusive_min_vector_helper_float4  : WgSnippetSpecialized<"fmin", work_group_scan_exclusive_function_op_vector4.snippet>;
def snippet_work_group_scan_exclusive_min_vector_helper_float8  : WgSnippetSpecialized<"fmin", work_group_scan_exclusive_function_op_vector8.snippet>;
def snippet_work_group_scan_exclusive_min_vector_helper_float16 : WgSnippetSpecialized<"fmin", work_group_scan_exclusive_function_op_vector16.snippet>;
def snippet_work_group_scan_exclusive_max_vector_helper_int4    : WgSnippetSpecialized<"max",  work_group_scan_exclusive_function_op_vector4.snippet>;
def snippet_work_group_scan_exclusive_max_vector_helper_int8    : WgSnippetSpecialized<"max",  work_group_scan_exclusive_function_op_vector8.snippet>;
def snippet_work_group_scan_exclusive_max_vector_helper_int16   : WgSnippetSpecialized<"max",  work_group_scan_exclusive_function_op_vector16.snippet>;
def snippet_work_group_scan_exclusive_max_vector_helper_float4  : WgSnippetSpecialized<"fmax", work_group_scan_exclusive_function_op_vector4.snippet>;
def snippet_work_group_scan_exclusive_max_vector_helper_float8  : WgSnippetSpecialized<"fmax", work_group_scan_exclusive_function_op_vector8.snippet>;
def snippet_work_group_scan_exclusive_max_vector_helper_float16 : WgSnippetSpecialized<"fmax", work_group_scan_exclusive_function_op_vector16.snippet>;
def snippet_work_group_scan_inclusive_add_vector_helper_gen4    : WgSnippetSpecialized<"work_group_add_util", work_group_scan_inclusive_function_op_vector4.snippet>;
def snippet_work_group_scan_inclusive_add_vector_helper_gen8    : WgSnippetSpecialized<"work_group_add_util", work_group_scan_inclusive_function_op_vector8.snippet>;
def snippet_work_group_scan_inclusive_add_vector_helper_gen16   : WgSnippetSpecialized<"work_group_add_util", work_group_scan_inclusive_function_op_vector16.snippet>;
def snippet_work_group_scan_inclusive_min_vector_helper_int4    : WgSnippetSpecialized<"min",  work_group_scan_inclusive_function_op_vector4.snippet>;
def snippet_work_group_scan_inclusive_min_vector_helper_int8    : WgSnippetSpecialized<"min",  work_group_scan_inclusive_function_op_vector8.snippet>;
def snippet_work_group_scan_inclusive_min_vector_helper_int16   : WgSnippetSpecialized<"min",  work_group_scan_inclusive_function_op_vector16.snippet>;
def snippet_work_group_scan_inclusive_min_vector_helper_float4  : WgSnippetSpecialized<"fmin", work_group_scan_inclusive_function_op_vector4.snippet>;
def snippet_work_group_scan_inclusive_min_vector_helper_float8  : WgSnippetSpecialized<"fmin", work_group_scan_inclusive_function_op_vector8.snippet>;
def snippet_work_group_scan_inclusive_min_vector_helper_float16 : WgSnippetSpecialized<"fmin", work_group_scan_inclusive_function_op_vector16.snippet>;
def snippet_work_group_scan_inclusive_max_vector_helper_int4    : WgSnippetSpecialized<"max",  work_group_scan_inclusive_function_op_vector4.snippet>;
def snippet_work_group_scan_inclusive_max_vector_helper_int8    : WgSnippetSpecialized<"max",  work_group_scan_inclusive_function_op_vector8.snippet>;
def snippet_work_group_scan_inclusive_max_vector_helper_int16   : WgSnippetSpecialized<"max",  work_group_scan_inclusive_function_op_vector16.snippet>;
def snippet_work_group_scan_inclusive_max_vector_helper_float4  : WgSnippetSpecialized<"fmax", work_group_scan_inclusive_function_op_vector4.snippet>;
def snippet_work_group_scan_inclusive_max_vector_helper_float8  : WgSnippetSpecialized<"fmax", work_group_scan_inclusive_function_op_vector8.snippet>;
def snippet_work_group_scan_inclusive_max_vector_helper_float16 : WgSnippetSpecialized<"fmax", work_group_scan_inclusive_function_op_vector16.snippet>;

// scalar versions of Scan_add/min/max functions
def snippet_work_group_scan_exclusive_add_scalar_helper_gen   : WgSnippetSpecialized<"work_group_add_util", work_group_scan_exclusive_function_op_scalar.snippet>;
def snippet_work_group_scan_exclusive_min_scalar_helper_int   : WgSnippetSpecialized<"min",                 work_group_scan_exclusive_function_op_scalar.snippet>;
def snippet_work_group_scan_exclusive_min_scalar_helper_float : WgSnippetSpecialized<"fmin",                work_group_scan_exclusive_function_op_scalar.snippet>;
def snippet_work_group_scan_exclusive_max_scalar_helper_int   : WgSnippetSpecialized<"max",                 work_group_scan_exclusive_function_op_scalar.snippet>;
def snippet_work_group_scan_exclusive_max_scalar_helper_float : WgSnippetSpecialized<"fmax",                work_group_scan_exclusive_function_op_scalar.snippet>;
def snippet_work_group_scan_inclusive_add_scalar_helper_gen   : WgSnippetSpecialized<"work_group_add_util", work_group_inclusive_function_op_scalar.snippet>;
def snippet_work_group_scan_inclusive_min_scalar_helper_int   : WgSnippetSpecialized<"min",                 work_group_inclusive_function_op_scalar.snippet>;
def snippet_work_group_scan_inclusive_min_scalar_helper_float : WgSnippetSpecialized<"fmin",                work_group_inclusive_function_op_scalar.snippet>;
def snippet_work_group_scan_inclusive_max_scalar_helper_int   : WgSnippetSpecialized<"max",                 work_group_inclusive_function_op_scalar.snippet>;
def snippet_work_group_scan_inclusive_max_scalar_helper_float : WgSnippetSpecialized<"fmax",                work_group_inclusive_function_op_scalar.snippet>;

// finalization functions
def snippet_work_group_all_finalize_helper_int4           : WgSnippetSpecialized<"work_group_all_util", work_group_finalize_function_op4.snippet>;
def snippet_work_group_all_finalize_helper_int8           : WgSnippetSpecialized<"work_group_all_util", work_group_finalize_function_op8.snippet>;
def snippet_work_group_all_finalize_helper_int16          : WgSnippetSpecialized<"work_group_all_util", work_group_finalize_function_op16.snippet>;
def snippet_work_group_any_finalize_helper_int4           : WgSnippetSpecialized<"work_group_any_util", work_group_finalize_function_op4.snippet>;
def snippet_work_group_any_finalize_helper_int8           : WgSnippetSpecialized<"work_group_any_util", work_group_finalize_function_op8.snippet>;
def snippet_work_group_any_finalize_helper_int16          : WgSnippetSpecialized<"work_group_any_util", work_group_finalize_function_op16.snippet>;
def snippet_work_group_reduce_add_finalize_helper_gen4    : WgSnippetSpecialized<"work_group_add_util", work_group_finalize_function_op4.snippet>;
def snippet_work_group_reduce_add_finalize_helper_gen8    : WgSnippetSpecialized<"work_group_add_util", work_group_finalize_function_op8.snippet>;
def snippet_work_group_reduce_add_finalize_helper_gen16   : WgSnippetSpecialized<"work_group_add_util", work_group_finalize_function_op16.snippet>;
def snippet_work_group_reduce_min_finalize_helper_int4    : WgSnippetSpecialized<"min",                 work_group_finalize_function_op4.snippet>;
def snippet_work_group_reduce_min_finalize_helper_int8    : WgSnippetSpecialized<"min",                 work_group_finalize_function_op8.snippet>;
def snippet_work_group_reduce_min_finalize_helper_int16   : WgSnippetSpecialized<"min",                 work_group_finalize_function_op16.snippet>;
def snippet_work_group_reduce_min_finalize_helper_float4  : WgSnippetSpecialized<"fmin",                work_group_finalize_function_op4.snippet>;
def snippet_work_group_reduce_min_finalize_helper_float8  : WgSnippetSpecialized<"fmin",                work_group_finalize_function_op8.snippet>;
def snippet_work_group_reduce_min_finalize_helper_float16 : WgSnippetSpecialized<"fmin",                work_group_finalize_function_op16.snippet>;
def snippet_work_group_reduce_max_finalize_helper_int4    : WgSnippetSpecialized<"max",                 work_group_finalize_function_op4.snippet>;
def snippet_work_group_reduce_max_finalize_helper_int8    : WgSnippetSpecialized<"max",                 work_group_finalize_function_op8.snippet>;
def snippet_work_group_reduce_max_finalize_helper_int16   : WgSnippetSpecialized<"max",                 work_group_finalize_function_op16.snippet>;
def snippet_work_group_reduce_max_finalize_helper_float4  : WgSnippetSpecialized<"fmax",                work_group_finalize_function_op4.snippet>;
def snippet_work_group_reduce_max_finalize_helper_float8  : WgSnippetSpecialized<"fmax",                work_group_finalize_function_op8.snippet>;
def snippet_work_group_reduce_max_finalize_helper_float16 : WgSnippetSpecialized<"fmax",                work_group_finalize_function_op16.snippet>;
