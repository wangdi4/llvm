// Expand
foreach name = ["acos", "acosh", "acospi", "asin", "asinh", "asinpi", "atan",
    "atan2", "atanh", "atanpi", "atan2pi", "cbrt", "ceil", "cos", "cosh",
    "cospi", "erfc", "erf", "exp", "exp2", "exp10", "expm1", "floor",
    "fma", "fmod", "fract_pas", "fract_las", "fract_gas", "fract_gen",
    "fract_pas2", "frexp_pas", "frexp_las", "frexp_gas", "frexp_gen", "hypot",
    "ilogb", "ldexp", "lgamma", "lgamma_r_pas", "lgamma_r_las", "lgamma_r_gas",
    "lgamma_r_gen", "log", "log2", "log10", "log1p", "logb", "modf_pas",
    "modf_las", "modf_gas", "modf_gen", "modf_pas2", "nextafter", "pow", "pown",
    "powr", "remainder", "remquo_pas", "remquo_las", "remquo_gas", "remquo_gen",
    "rint", "rootn", "round", "rsqrt", "sin", "sincos_pas", "sincos_las",
    "sincos_gas", "sincos_gen", "sincos_pas2", "sinh", "sinpi", "sqrt", "tan",
    "tanh", "tanpi", "tgamma", "trunc",

    "native_acos", "native_acosh", "native_acospi", "native_asin", "native_asinh",
    "native_asinpi", "native_atan", "native_atan2", "native_atanh", "native_atanpi",
    "native_atan2pi", "native_cbrt", "native_cos", "native_cosh", "native_cospi",
    "native_divide", "native_erfc", "native_erf", "native_exp", "native_exp2",
    "native_exp10", "native_expm1", "native_fract_pas", "native_fract_las",
    "native_fract_gas", "native_fract_gen", "native_fract_pas2",
    "native_ilogb", "native_log", "native_log2", "native_log10", "native_log1p",
    "native_pow", "native_pown", "native_powr", "native_rcbrt", "native_recip",
    "native_rootn", "native_rsqrt", "native_sin", "native_sinh", "native_sinpi",
    "native_sincos_pas", "native_sincos_las", "native_sincos_gas",
    "native_sincos_gen", "native_sincos_pas2", "native_sqrt",
    "native_tan", "native_tanh", "native_tanpi"] in {
  defvar builtin = !cast<OclBuiltin>(name);

  defvar v32v64_types = [v64f32, v32f64, v64f64];
  defvar v32v64_types_all = ExpandTypesByVFAndFlatten<[v1f32, v1f64], [32, 64]>.Tout;
  defvar types = !cond(
      !or(!ne(!find(name, "fract_"), -1), !eq(name, "ceil"), !eq(name, "floor"),
          !eq(name, "fma"), !eq(name, "sqrt"), !eq(name, "native_rsqrt")): v32v64_types_all,
      true: v32v64_types);

  defvar num_args = !size(builtin.Ins);
  defvar expand = !cond(
      !ne(!find(name, "_pas2"), -1): Expand_2Ptr_NoRet,
      !or(!ne(!find(name, "fract_"), -1), !ne(!find(name, "modf_"), -1), !ne(!find(name, "sincos_"), -1)): Expand_2Ptr,
      !or(!ne(!find(name, "frexp_"), -1), !ne(!find(name, "lgamma_r_"), -1)) : Expand_frexp_lgamma,
      !ne(!find(name, "remquo_"), -1): Expand_remquo,
      !eq(num_args, 1): Expand1,
      !eq(num_args, 2): Expand2,
      !eq(num_args, 3): Expand3);

  def name # _expand_v32v64 : OclBuiltinImpl<builtin, types, 0, expand>;
}

// SVML calls
foreach name = ["acos", "acosh", "acospi", "asin", "asinh", "asinpi", "atan",
    "atan2", "atanh", "atanpi", "atan2pi", "cbrt", "cos", "cosh", "cospi",
    "erfc", "erf", "exp", "exp2", "exp10", "expm1", "fmod", "hypot", "ilogb",
    "ldexp", "lgamma", "log", "log2", "log10", "log1p", "logb", "nextafter", 
    "pow", "pown", "powr", "remainder", "rint", "rootn", "round", "rsqrt", 
    "sin", "sinh", "sinpi", "tan", "tanh", "tanpi", "tgamma"] in {
  defvar builtin = !cast<OclBuiltin>(name);
  defvar types = !cond(!eq(name, "ilogb") : [v8f64, v32f32],
                       true: [v8f64, v16f32, v32f32]);
  defvar num_args = !size(builtin.Ins);
  defvar svml_code1 = [{
    return __ocl_svml_$Target_$NAME$SVMLSuffix($Arg0VarName);
  }];
  defvar svml_code2 = [{
    return __ocl_svml_$Target_$NAME$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }];
  defvar svml_code = !cond(!eq(num_args, 1) : svml_code1, true: svml_code2);
  def name # _impl : OclBuiltinImpl<builtin, types, 0, svml_code, [["$NAME", name]]>;
}

// Native SVML calls
foreach name = ["acos", "acosh", "acospi", "asin",
    "asinh", "asinpi", "atan", "atan2", 
    "atanh", "atanpi", "atan2pi", "cbrt", 
    "cos", "cosh", "cospi", "erfc", "erf", 
    "exp", "exp2", "exp10", "expm1", "log", 
    "log2", "log10", "log1p", "pow", "pown", 
    "powr", "rcbrt", "rootn", "sin", "sinh", 
    "sinpi", "sqrt", "tan", "tanh", "tanpi"] in {
  defvar builtin = !cast<OclBuiltin>("native_" # name);
  defvar types = [v8f64, v16f32, v32f32];
  defvar num_args = !size(builtin.Ins);
  defvar svml_code1 = [{
    return __ocl_svml_$Target_$NAME$SVMLSuffix_native($Arg0VarName);
  }];
  defvar svml_code2 = [{
    return __ocl_svml_$Target_$NAME$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }];
  defvar svml_code = !cond(!eq(num_args, 1) : svml_code1, true: svml_code2);
  def name # _native_impl : OclBuiltinImpl<builtin, types, 0, svml_code, [["$NAME", name]]>;
}
