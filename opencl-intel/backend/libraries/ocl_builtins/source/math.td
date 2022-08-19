// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

defvar math_f16_types = ExpandTypeByVF<v1f16, [1, 2, 3, 4, 8, 16, 32, 64]>.Tout;
defvar math_f32_types = ExpandTypeByVF<v1f32, [1, 2, 3, 4, 8, 16, 32, 64]>.Tout;
defvar math_f16_vectorize_types = ExpandTypesByVFAndFlatten<[v1f16], [4, 8, 16, 32, 64]>.Tout;

// --- Math Functions ---
// https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_C.html#math-functions
//
// Prototypes
let Types = math_f16_types # math_f32_types, // f32 types are added for forward declarations, so that we can promote f16 to f32 calls.
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = true in {
// Builtins that use Spec-defined vector versions as vector variants.
defset list<OclBuiltin> math_builtins_with_general_vect_info = {
  // Builtins that are implemented with general __ocl_svml calls.
  defset list<OclBuiltin> math_generic_builtins = {
    def acos_f16 : OclBuiltin<"acos", (outs gentype:$ret), (ins gentype:$x)>;
    def acosh_f16 : OclBuiltin<"acosh", (outs gentype:$ret), (ins gentype:$x)>;
    def acospi_f16 : OclBuiltin<"acospi", (outs gentype:$ret), (ins gentype:$x)>;
    def asin_f16 : OclBuiltin<"asin", (outs gentype:$ret), (ins gentype:$x)>;
    def asinh_f16 : OclBuiltin<"asinh", (outs gentype:$ret), (ins gentype:$x)>;
    def asinpi_f16 : OclBuiltin<"asinpi", (outs gentype:$ret), (ins gentype:$x)>;
    def atan_f16 : OclBuiltin<"atan", (outs gentype:$ret), (ins gentype:$y_over_x)>;
    def atan2_f16 : OclBuiltin<"atan2", (outs gentype:$ret), (ins gentype:$y, gentype:$x)>;
    def atanh_f16 : OclBuiltin<"atanh", (outs gentype:$ret), (ins gentype:$x)>;
    def atanpi_f16 : OclBuiltin<"atanpi", (outs gentype:$ret), (ins gentype:$x)>;
    def atan2pi_f16 : OclBuiltin<"atan2pi", (outs gentype:$ret), (ins gentype:$y, gentype:$x)>;
    def cbrt_f16 : OclBuiltin<"cbrt", (outs gentype:$ret), (ins gentype:$x)>;
    def ceil_f16 : OclBuiltin<"ceil", (outs gentype:$ret), (ins gentype:$x)>;
    def copysign_f16 : OclBuiltin<"copysign", (outs gentype:$ret), (ins gentype:$x, gentype:$y)>;
    def cos_f16 : OclBuiltin<"cos", (outs gentype:$ret), (ins gentype:$x)>;
    def cosh_f16 : OclBuiltin<"cosh", (outs gentype:$ret), (ins gentype:$x)>;
    def cospi_f16 : OclBuiltin<"cospi", (outs gentype:$ret), (ins gentype:$x)>;
    def erfc_f16 : OclBuiltin<"erfc", (outs gentype:$ret), (ins gentype:$x)>;
    def erf_f16 : OclBuiltin<"erf", (outs gentype:$ret), (ins gentype:$x)>;
    def exp_f16 : OclBuiltin<"exp", (outs gentype:$ret), (ins gentype:$x)>;
    def exp2_f16 : OclBuiltin<"exp2", (outs gentype:$ret), (ins gentype:$x)>;
    def exp10_f16 : OclBuiltin<"exp10", (outs gentype:$ret), (ins gentype:$x)>;
    def expm1_f16 : OclBuiltin<"expm1", (outs gentype:$ret), (ins gentype:$x)>;
    // def fabs_f16 : OclBuiltin<"fabs", (outs gentype:$ret), (ins gentype:$x)>;
    def fdim_f16 : OclBuiltin<"fdim", (outs gentype:$ret), (ins gentype:$x, gentype:$y)>;
    def floor_f16 : OclBuiltin<"floor", (outs gentype:$ret), (ins gentype:$x)>;
    def fma_f16 : OclBuiltin<"fma", (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c)>;
    def fmax_f16 : OclBuiltin<"fmax", (outs gentype:$ret), (ins gentype:$x, gentype:$y)>;
    def fmax_common_f16 : OclBuiltin<"fmax_common", (outs gentype:$ret), (ins gentype:$x, gentype:$y)>;
    def fmin_f16 : OclBuiltin<"fmin", (outs gentype:$ret), (ins gentype:$x, gentype:$y)>;
    def fmin_common_f16 : OclBuiltin<"fmin_common", (outs gentype:$ret), (ins gentype:$x, gentype:$y)>;
    def fmod_f16 : OclBuiltin<"fmod", (outs gentype:$ret), (ins gentype:$x, gentype:$y)>;
    // def fract_pas2_f16 : OclBuiltin<"fract_ret2ptr", (outs), (ins gentype:$x, pgentype:$ret0 ,pgentype:$ret1)>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
    def hypot_f16 : OclBuiltin<"hypot", (outs gentype:$ret), (ins gentype:$x, gentype:$y)>;
    def ilogb_f16 : OclBuiltin<"ilogb", (outs i32gentype:$ret), (ins gentype:$x)>;
    // def ldexp_s_f16 : OclBuiltin<"ldexp", (outs gentype:$ret), (ins gentype:$x, si32gentype:$k)>, Except<[v1f16, v1f32]>;
    def lgamma_f16 : OclBuiltin<"lgamma", (outs gentype:$ret), (ins gentype:$x)>;
    def log_f16 : OclBuiltin<"log", (outs gentype:$ret), (ins gentype:$x)>;
    def log2_f16 : OclBuiltin<"log2", (outs gentype:$ret), (ins gentype:$x)>;
    def log10_f16 : OclBuiltin<"log10", (outs gentype:$ret), (ins gentype:$x)>;
    def log1p_f16 : OclBuiltin<"log1p", (outs gentype:$ret), (ins gentype:$x)>;
    def logb_f16 : OclBuiltin<"logb", (outs gentype:$ret), (ins gentype:$x)>;
    def maxmag_f16 : OclBuiltin<"maxmag", (outs gentype:$ret), (ins gentype:$x, gentype:$y)>;
    def minmag_f16 : OclBuiltin<"minmag", (outs gentype:$ret), (ins gentype:$x, gentype:$y)>;
    // def modf_pas2_f16 : OclBuiltin<"modf_ret2ptr", (outs), (ins gentype:$x, pgentype:$sinval ,pgentype:$cosval)>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
    def nextafter_f16 : OclBuiltin<"nextafter", (outs gentype:$ret), (ins gentype:$x, gentype:$y)>;
    def pow_f16 : OclBuiltin<"pow", (outs gentype:$ret), (ins gentype:$x, gentype:$y)>;
    def powr_f16 : OclBuiltin<"powr", (outs gentype:$ret), (ins gentype:$x, gentype:$y)>;
    def remainder_f16 : OclBuiltin<"remainder", (outs gentype:$ret), (ins gentype:$x, gentype:$y)>;
    def rint_f16 : OclBuiltin<"rint", (outs gentype:$ret), (ins gentype:$x)>;
    def round_f16 : OclBuiltin<"round", (outs gentype:$ret), (ins gentype:$x)>;
    def rsqrt_f16 : OclBuiltin<"rsqrt", (outs gentype:$ret), (ins gentype:$x)>;
    def sin_f16 : OclBuiltin<"sin", (outs gentype:$ret), (ins gentype:$x)>;
    // def sincos_pas2_f16 : OclBuiltin<"sincos_ret2ptr", (outs), (ins gentype:$x, pgentype:$sinval ,pgentype:$cosval)>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
    def sinh_f16 : OclBuiltin<"sinh", (outs gentype:$ret), (ins gentype:$x)>;
    def sinpi_f16 : OclBuiltin<"sinpi", (outs gentype:$ret), (ins gentype:$x)>;
    def sqrt_f16 : OclBuiltin<"sqrt", (outs gentype:$ret), (ins gentype:$x)>;
    def tan_f16 : OclBuiltin<"tan", (outs gentype:$ret), (ins gentype:$x)>;
    def tanh_f16 : OclBuiltin<"tanh", (outs gentype:$ret), (ins gentype:$x)>;
    def tanpi_f16 : OclBuiltin<"tanpi", (outs gentype:$ret), (ins gentype:$x)>;
    def tgamma_f16 : OclBuiltin<"tgamma", (outs gentype:$ret), (ins gentype:$x)>;
    def trunc_f16 : OclBuiltin<"trunc", (outs gentype:$ret), (ins gentype:$x)>;
  } // defset math_generic_builtins

  def mad_f16 : OclBuiltin<"mad", (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c)>;
  def nan_f16 : OclBuiltin<"nan", (outs gentype:$ret), (ins ugentype:$nancode)>;

  defset list<OclBuiltin> math_builtins_with_i32gentype_arg1 = {
    def ldexp_f16 : OclBuiltin<"ldexp", (outs gentype:$ret), (ins gentype:$x, i32gentype:$k)>;
    def pown_f16 : OclBuiltin<"pown", (outs gentype:$ret), (ins gentype:$x, i32gentype:$y)>;
    def rootn_f16 : OclBuiltin<"rootn", (outs gentype:$ret), (ins gentype:$x, i32gentype:$y)>;
  } // defset math_builtins_with_i32gentype_arg1
} // defset math_builtins_with_general_vect_info

// Builtins that do not need VectInfos.
defset list<OclBuiltin> math_builtins_fmax_fmin_sgentype_arg1 = {
  def fmax_s_f16 : OclBuiltin<"fmax", (outs gentype:$ret), (ins gentype:$x, sgentype:$y)>, Except<[v1f16, v1f32]>;
  def fmin_s_f16 : OclBuiltin<"fmin", (outs gentype:$ret), (ins gentype:$x, sgentype:$y)>, Except<[v1f16, v1f32]>;
} // defset math_builtins_fmax_fmin_sgentype_arg1
}

// Non-const prototypes
let Types = math_f16_types # math_f32_types,
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = true in {
// Builtins whose second arg is pgentype.
defset list<OclBuiltin> math_builtins_with_pgentype_arg1 = {
  def fract_pas_f16 : OclBuiltin<"fract", (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr)>, PRIVATE;
  def fract_las_f16 : OclBuiltin<"fract", (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr)>, LOCAL;
  def fract_gas_f16 : OclBuiltin<"fract", (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr)>, GLOBAL;
  def fract_gen_f16 : OclBuiltin<"fract", (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr)>;
  def modf_pas_f16 : OclBuiltin<"modf", (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr)>, PRIVATE;
  def modf_las_f16 : OclBuiltin<"modf", (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr)>, LOCAL;
  def modf_gas_f16 : OclBuiltin<"modf", (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr)>, GLOBAL;
  def modf_gen_f16 : OclBuiltin<"modf", (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr)>;
  def sincos_pas_f16 : OclBuiltin<"sincos", (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval)>, PRIVATE;
  def sincos_las_f16 : OclBuiltin<"sincos", (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval)>, LOCAL;
  def sincos_gas_f16 : OclBuiltin<"sincos", (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval)>, GLOBAL;
  def sincos_gen_f16 : OclBuiltin<"sincos", (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval)>;
} // defset math_builtins_with_pgentype_arg1

// Linear strided variants for math_builtins_with_pgentype_arg1
let Types = math_f16_vectorize_types in
defset list<OclBuiltin> math_builtins_with_pgentype_arg1_strided_variants = {
  def fract_pas_f16_strided : OclBuiltin<"fract", (outs gentype:$ret), (ins gentype:$x, psgentype:$iptr)>, PRIVATE;
  def fract_las_f16_strided : OclBuiltin<"fract", (outs gentype:$ret), (ins gentype:$x, psgentype:$iptr)>, LOCAL;
  def fract_gas_f16_strided : OclBuiltin<"fract", (outs gentype:$ret), (ins gentype:$x, psgentype:$iptr)>, GLOBAL;
  def fract_gen_f16_strided : OclBuiltin<"fract", (outs gentype:$ret), (ins gentype:$x, psgentype:$iptr)>;
  def modf_pas_f16_strided : OclBuiltin<"modf", (outs gentype:$ret), (ins gentype:$x, psgentype:$iptr)>, PRIVATE;
  def modf_las_f16_strided : OclBuiltin<"modf", (outs gentype:$ret), (ins gentype:$x, psgentype:$iptr)>, LOCAL;
  def modf_gas_f16_strided : OclBuiltin<"modf", (outs gentype:$ret), (ins gentype:$x, psgentype:$iptr)>, GLOBAL;
  def modf_gen_f16_strided : OclBuiltin<"modf", (outs gentype:$ret), (ins gentype:$x, psgentype:$iptr)>;
  def sincos_pas_f16_strided : OclBuiltin<"sincos", (outs gentype:$ret), (ins gentype:$x, psgentype:$cosval)>, PRIVATE;
  def sincos_las_f16_strided : OclBuiltin<"sincos", (outs gentype:$ret), (ins gentype:$x, psgentype:$cosval)>, LOCAL;
  def sincos_gas_f16_strided : OclBuiltin<"sincos", (outs gentype:$ret), (ins gentype:$x, psgentype:$cosval)>, GLOBAL;
  def sincos_gen_f16_strided : OclBuiltin<"sincos", (outs gentype:$ret), (ins gentype:$x, psgentype:$cosval)>;
} // defset math_builtins_with_pgentype_arg1_strided_variants

// Builtins whose last arg is pi32gentype
defset list<OclBuiltin> math_builtins_with_pi32gentype_lastarg = {
  def frexp_pas_f16 : OclBuiltin<"frexp", (outs gentype:$ret), (ins gentype:$x, pi32gentype:$iptr)>, PRIVATE;
  def frexp_las_f16 : OclBuiltin<"frexp", (outs gentype:$ret), (ins gentype:$x, pi32gentype:$iptr)>, LOCAL;
  def frexp_gas_f16 : OclBuiltin<"frexp", (outs gentype:$ret), (ins gentype:$x, pi32gentype:$iptr)>, GLOBAL;
  def frexp_gen_f16 : OclBuiltin<"frexp", (outs gentype:$ret), (ins gentype:$x, pi32gentype:$iptr)>;
  def lgamma_r_pas_f16 : OclBuiltin<"lgamma_r", (outs gentype:$ret), (ins gentype:$x, pi32gentype:$signp)>, PRIVATE;
  def lgamma_r_las_f16 : OclBuiltin<"lgamma_r", (outs gentype:$ret), (ins gentype:$x, pi32gentype:$signp)>, LOCAL;
  def lgamma_r_gas_f16 : OclBuiltin<"lgamma_r", (outs gentype:$ret), (ins gentype:$x, pi32gentype:$signp)>, GLOBAL;
  def lgamma_r_gen_f16 : OclBuiltin<"lgamma_r", (outs gentype:$ret), (ins gentype:$x, pi32gentype:$signp)>;
  def remquo_pas_f16 : OclBuiltin<"remquo", (outs gentype:$ret), (ins gentype:$x, gentype:$y, pi32gentype:$quo)>, PRIVATE;
  def remquo_las_f16 : OclBuiltin<"remquo", (outs gentype:$ret), (ins gentype:$x, gentype:$y, pi32gentype:$quo)>, LOCAL;
  def remquo_gas_f16 : OclBuiltin<"remquo", (outs gentype:$ret), (ins gentype:$x, gentype:$y, pi32gentype:$quo)>, GLOBAL;
  def remquo_gen_f16 : OclBuiltin<"remquo", (outs gentype:$ret), (ins gentype:$x, gentype:$y, pi32gentype:$quo)>;
} // defset math_builtins_with_pi32gentype_lastarg

// Linear strided varaints for math_builtins_with_pi32gentype_lastarg
let Types = math_f16_vectorize_types in
defset list<OclBuiltin> math_builtins_with_pi32gentype_lastarg_strided_variants = {
  def frexp_pas_f16_strided : OclBuiltin<"frexp", (outs gentype:$ret), (ins gentype:$x, pv1i32:$iptr)>, PRIVATE;
  def frexp_las_f16_strided : OclBuiltin<"frexp", (outs gentype:$ret), (ins gentype:$x, pv1i32:$iptr)>, LOCAL;
  def frexp_gas_f16_strided : OclBuiltin<"frexp", (outs gentype:$ret), (ins gentype:$x, pv1i32:$iptr)>, GLOBAL;
  def frexp_gen_f16_strided : OclBuiltin<"frexp", (outs gentype:$ret), (ins gentype:$x, pv1i32:$iptr)>;
  def lgamma_r_pas_f16_strided : OclBuiltin<"lgamma_r", (outs gentype:$ret), (ins gentype:$x, pv1i32:$signp)>, PRIVATE;
  def lgamma_r_las_f16_strided : OclBuiltin<"lgamma_r", (outs gentype:$ret), (ins gentype:$x, pv1i32:$signp)>, LOCAL;
  def lgamma_r_gas_f16_strided : OclBuiltin<"lgamma_r", (outs gentype:$ret), (ins gentype:$x, pv1i32:$signp)>, GLOBAL;
  def lgamma_r_gen_f16_strided : OclBuiltin<"lgamma_r", (outs gentype:$ret), (ins gentype:$x, pv1i32:$signp)>;
  def remquo_pas_f16_strided : OclBuiltin<"remquo", (outs gentype:$ret), (ins gentype:$x, gentype:$y, pv1i32:$quo)>, PRIVATE;
  def remquo_las_f16_strided : OclBuiltin<"remquo", (outs gentype:$ret), (ins gentype:$x, gentype:$y, pv1i32:$quo)>, LOCAL;
  def remquo_gas_f16_strided : OclBuiltin<"remquo", (outs gentype:$ret), (ins gentype:$x, gentype:$y, pv1i32:$quo)>, GLOBAL;
  def remquo_gen_f16_strided : OclBuiltin<"remquo", (outs gentype:$ret), (ins gentype:$x, gentype:$y, pv1i32:$quo)>;
} // defset math_builtins_with_pi32gentype_lastarg_strided_variants
}

defvar promote_f16_to_f32_code = [{
  return convert_$ReturnType($Func($ARGLIST));
}];

defvar call_svml_code = [{
  return __ocl_svml_$TargetShortName_$Func$SVMLSuffix($ARGLIST);
}];

defvar expand_code = [{
  $ReturnType _res;
  _res.lo = $Func($LOARGLIST);
  _res.hi = $Func($HIARGLIST);
  return _res;
}];

// Implementations
foreach bi = math_generic_builtins in {
  // [SHARED] vXf16: convert f16 args to f32 and call f32 implementations.
  defvar indices = Range<0, !size(bi.Ins)>.Tout;
  defvar promoted_args = RangeStr<"convert_$Arg#f32gentype($Arg#VarName)", indices>.Tout;
  def : OclBuiltinImpl<bi, math_f16_types, false, promote_f16_to_f32_code, [["$ARGLIST", !interleave(promoted_args, ", ")]]>, SHARED;

  // [AMX] vXf16 (X <= 32): call __ocl_svml func.
  // For fmax_common and fmin_common, just call __ocl_svml_*_fmax/fmin
  defvar func_macro = ["$Func", !subst("_common", "", bi.Name)]; // Removing the "_common" suffix for fmax_common/fmin_common
  defvar args = RangeStr<"$Arg#VarName", indices>.Tout;
  let Types = !filter(ty, math_f16_types, !le(ty.VecLength, 32)) in
  def : OclBuiltinImpl<bi, [], false, call_svml_code, [func_macro, ["$ARGLIST", !interleave(args, ", ")]]>, AMX;

  // [AMX] vXf16 (X > 32): expand to two v[X/2]f16 calls.
  defvar expanded_lo_args = RangeStr<"$Arg#VarName.lo", indices>.Tout;
  defvar expanded_hi_args = RangeStr<"$Arg#VarName.hi", indices>.Tout;
  let Types = !filter(ty, math_f16_types, !gt(ty.VecLength, 32)) in
  def : OclBuiltinImpl<bi, [], false, expand_code, [["$LOARGLIST", !interleave(expanded_lo_args, ", ")], ["$HIARGLIST", !interleave(expanded_hi_args, ", ")]]>, AMX;

  // TODO: (when masked variants are added)
  // [AMX] v3f16: call masked v4f16
}

// Specialized implementations
// mad: returns a*b+c
def : OclBuiltinImpl<mad_f16, math_f16_types, false, mad_code>, SHARED;

// nan: returns 0b0111_1100_0000_0000 | nancode
def : OclBuiltinImpl<nan_f16, math_f16_types, false, [{
  return __builtin_astype(($Arg0Type)($Arg0VarName | ($Arg0Type)0x7C00), $ReturnType);
}]>, SHARED;

// gentype func(gentype, gentype *)
foreach bi = math_builtins_with_pgentype_arg1 in {
  // [SHARED] vXf16: convert f16 args to f32 and call f32 implementations.
  def : OclBuiltinImpl<bi, math_f16_types, false, [{
    $Arg0f32gentype _expand;
    $ReturnType _res = convert_$ReturnType($Func(convert_$Arg0f32gentype($Arg0VarName), &_expand));
    *$Arg1VarName = convert_$Arg0Type(_expand);
    return _res;
  }]>, SHARED;

  // [AMX] vXf16 (X <= 32): call __ocl_svml func.
  let Types = !filter(ty, math_f16_types, !le(ty.VecLength, 32)) in
  def : OclBuiltinImpl<bi, [], false, call_svml_code, [["$ARGLIST", "$Arg0VarName, $Arg1VarName"]]>, AMX;

  // [AMX] vXf16 (X > 32): expand to two v[X/2]f16 calls.
  let Types = !filter(ty, math_f16_types, !gt(ty.VecLength, 32)) in
  def : OclBuiltinImpl<bi, [], false, [{
    $ReturnType _res;
    $ExpandArg0Type _lo, _hi;
    _res.lo = $Func($Arg0VarName.lo, &_lo);
    _res.hi = $Func($Arg0VarName.hi, &_hi);
    *$Arg1VarName = ($Arg0Type)(_lo, _hi);
    return _res;
  }]>, AMX;

  // TODO: (when masked variants are added)
  // [AMX] v3f16: call masked v4f16
}

// Linear strided variants of 'gentype func(gentype, gentype *)'
foreach bi = math_builtins_with_pgentype_arg1_strided_variants in {
  defvar linear_strided_code = [{
    // The access to $Arg1VarName is guaranteed to be linear-strided.
    return $Func($Arg0VarName, ($Arg0Type *)$Arg1VarName);
  }];
  def : OclBuiltinImpl<bi, math_f16_vectorize_types, false, linear_strided_code>, SHARED;
  // Explicitly put a copy of linear strided impl in AMX rtl, so that the native
  // SVML impl can be inlined.
  def : OclBuiltinImpl<bi, math_f16_vectorize_types, false, linear_strided_code>, AMX;
}

// gentype func(gentype, [gentype, ]i32gentype *)
foreach bi = math_builtins_with_pi32gentype_lastarg in {
  // [SHARED] vXf16: convert Arg0 to f32 and call f32 implementations.
  // All args except last arg (pi32gentype) need promotion.
  defvar pi32_index = !sub(!size(bi.Ins), 1);
  defvar pi32_arg_name = "$Arg" # pi32_index # "VarName";
  defvar gentype_indices = Range<0, pi32_index>.Tout;
  defvar promoted_args = RangeStr<"convert_$Arg#f32gentype($Arg#VarName)", gentype_indices>.Tout # [pi32_arg_name];
  def : OclBuiltinImpl<bi, math_f16_types, false, promote_f16_to_f32_code, [["$ARGLIST", !interleave(promoted_args, ", ")]]>, SHARED;

  // [AMX] vXf16 (X <= 32): call __ocl_svml func.
  // SVML lib uses 'lgammar' as the name of 'lgamma_r'.
  defvar func_macro = ["$Func", !if(!eq(bi.Name, "lgamma_r"), "lgammar", bi.Name)];
  defvar indices = Range<0, !size(bi.Ins)>.Tout;
  defvar args = RangeStr<"$Arg#VarName", indices>.Tout;
  let Types = !filter(ty, math_f16_types, !le(ty.VecLength, 32)) in
  def : OclBuiltinImpl<bi, [], false, call_svml_code, [func_macro, ["$ARGLIST", !interleave(args, ", ")]]>, AMX;

  // [AMX] vXf16 (X > 32): expand to two v[X/2]f16 calls.
  defvar expanded_lo_args = RangeStr<"$Arg#VarName.lo", gentype_indices>.Tout;
  defvar expanded_hi_args = RangeStr<"$Arg#VarName.hi", gentype_indices>.Tout;
  let Types = !filter(ty, math_f16_types, !gt(ty.VecLength, 32)) in
  def : OclBuiltinImpl<bi, [], false, [{
    $ReturnType _res;
    int$LoSuffix _lo, _hi;
    _res.lo = $Func($LOARGLIST, &_lo);
    _res.hi = $Func($HIARGLIST, &_hi);
    *$LASTARG = (int$VecLength)(_lo, _hi);
    return _res;
  }], [["$LOARGLIST", !interleave(expanded_lo_args, ", ")], ["$HIARGLIST", !interleave(expanded_hi_args, ", ")], ["$LASTARG", pi32_arg_name]]>, AMX;

  // TODO: (when masked variants are added)
  // [AMX] v3f16: call masked v4f16
}

// Linear strided variants of 'gentype func(gentype, [gentype, ]i32gentype *)'
foreach bi = math_builtins_with_pi32gentype_lastarg_strided_variants in {
  defvar pi32_index = !sub(!size(bi.Ins), 1);
  defvar pi32_arg_name = "$Arg" # pi32_index # "VarName";
  defvar gentype_indices = Range<0, pi32_index>.Tout;
  defvar gentype_args = RangeStr<"$Arg#VarName", gentype_indices>.Tout;
  defvar linear_strided_code = [{
    // The access to $Arg1VarName is guaranteed to be linear-strided.
    return $Func($ARGLIST, (int$VecLength *)$LASTARG);
  }];
  defvar macros = [["$ARGLIST", !interleave(gentype_args, ", ")], ["$LASTARG", pi32_arg_name]];
  def : OclBuiltinImpl<bi, math_f16_vectorize_types, false, linear_strided_code, macros>, SHARED;
  // Explicitly put a copy of linear strided impl in AMX rtl, so that the native
  // SVML impl can be inlined.
  def : OclBuiltinImpl<bi, math_f16_vectorize_types, false, linear_strided_code, macros>, AMX;
}

// gentype func(gentype, i32gentype)
foreach bi = math_builtins_with_i32gentype_arg1 in {
  // [SHARED] vXf16: convert Arg0 to f32 and call f32 implementations.
  def : OclBuiltinImpl<bi, math_f16_types, false, promote_f16_to_f32_code, [["$ARGLIST", "convert_$Arg0f32gentype($Arg0VarName), $Arg1VarName"]]>, SHARED;

  // [AMX] vXf16 (X <= 32): call __ocl_svml func.
  let Types = !filter(ty, math_f16_types, !le(ty.VecLength, 32)) in
  def : OclBuiltinImpl<bi, [], false, call_svml_code, [["$ARGLIST", "$Arg0VarName, $Arg1VarName"]]>, AMX;

  // [AMX] vXf16 (X > 32): expand to two v[X/2]f16 calls.
  let Types = !filter(ty, math_f16_types, !gt(ty.VecLength, 32)) in
  def : OclBuiltinImpl<bi, [], false, expand_code, [["$LOARGLIST", "$Arg0VarName.lo, $Arg1VarName.lo"], ["$HIARGLIST", "$Arg0VarName.hi, $Arg1VarName.hi"]]>, AMX;

  // TODO: (when masked variants are added)
  // [AMX] v3f16: call masked v4f16
}

// gentype fmax/fmin(gentype, sgentype)
// Broadcast arg1 to vector type.
foreach bi = math_builtins_fmax_fmin_sgentype_arg1 in {
  defvar broadcast_code = [{
    return $Func($Arg0VarName, ($Arg0Type)$Arg1VarName);
  }];
  def : OclBuiltinImpl<bi, math_f16_types, false, broadcast_code>, SHARED;
  // Explicitly put a copy of broadcasting impl in AMX rtl, so that the native
  // SVML impl can be inlined.
  def : OclBuiltinImpl<bi, math_f16_types, false, broadcast_code>, AMX;
}

// VectInfos
foreach bi = math_builtins_with_general_vect_info in {
  def : VectInfo<[v1f16], bi, bi, bi, bi, bi, bi>;

  // TODO: add masked VectInfo
}

// Specialized VectInfos
// Linear strided for ptr arg
foreach bi = math_builtins_with_pgentype_arg1 in {
  defvar strided = !cast<OclBuiltin>(bi # "_strided");
  let Stride = 2 in // stride for f16 type
  def : VectInfo<[v1f16], bi, strided, strided, strided, strided, strided>;
}

foreach bi = math_builtins_with_pi32gentype_lastarg in {
  defvar strided = !cast<OclBuiltin>(bi # "_strided");
  let Stride = 4 in // stride for i32 type
  def : VectInfo<[v1f16], bi, strided, strided, strided, strided, strided>;
}
