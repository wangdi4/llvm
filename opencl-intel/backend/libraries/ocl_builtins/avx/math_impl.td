// SVML calls
foreach name = ["acos", "acosh", "acospi", "asin", "asinh", "asinpi", "atan",
    "atan2", "atanh", "atanpi", "atan2pi", "cbrt", "cos", "cosh", "cospi",
    "erfc", "erf", "exp", "exp2", "exp10", "expm1", "fmod",
    "frexp_pas", "frexp_las", "frexp_gas", "frexp_gen", "hypot",
    /*"ilogb",*/ "ldexp", "lgamma", "lgamma_r_pas", "lgamma_r_las", "lgamma_r_gas",
    "lgamma_r_gen", "log", "log2", "log10", "log1p", "logb",
    "modf_pas", "modf_las", "modf_gas", "modf_gen", "nextafter",
    "pow", "pown", "powr", "remainder", "remquo_pas", "remquo_las",
    "remquo_gas", "remquo_gen", "rootn", "rsqrt", "sin",
    "sincos_pas", "sincos_las", "sincos_gas", "sincos_gen",
    "sinh", "sinpi", "tan", "tanh", "tanpi", "tgamma"] in {
  defvar builtin = !cast<OclBuiltin>(name);
  // Some v16f64 svml builtins has runfail on avx2, so we don't enable them.
  defvar types = [v16f32, v8f64];
  defvar num_args = !size(builtin.Ins);
  defvar svml_code = !cond(!ne(!find(name, "frexp_"), -1): frexp_svml,
      !ne(!find(name, "lgamma_r_"), -1): lgamma_r_svml,
      !ne(!find(name, "modf_"), -1): modf_svml,
      !ne(!find(name, "remquo_"), -1): remquo_svml,
      !ne(!find(name, "sincos_"), -1): sincos_svml,
      !eq(num_args, 1): svml_code1,
      true: svml_code2);
  defvar svml_name = !cond(
      !ne(!find(name, "frexp_"), -1): "frexp",
      !ne(!find(name, "lgamma_r_"), -1): "lgamma_r",
      !ne(!find(name, "modf_"), -1): "modf",
      !ne(!find(name, "remquo_"), -1): "remquo",
      !ne(!find(name, "sincos_"), -1): "sincos",
      true: name);
  def name # _avx_impl : OclBuiltinImpl<builtin, types, 0, svml_code, [["$NAME", svml_name]]>;
}

// Relaxed SVML calls
foreach name = ["cos", "divide", "exp", "exp2", "exp10", "log", "log2", "pow",
    "sin", "tan", "sincos_pas", "sincos_las", "sincos_gas", "sincos_gen"] in {
  defvar builtin = !cast<OclBuiltin>(name # "_rm");
  defvar num_args = !size(builtin.Ins);
  defvar svml_code = !cond(!ne(!find(name, "sincos_"), -1): svml_rm_code_sincos,
      !eq(name, "divide"): svml_rm_code_divide,
      !eq(num_args, 1): svml_rm_code1, true: svml_rm_code2);
  def name # _rm_avx_impl : OclBuiltinImpl<builtin, [v16f32], 0, svml_code, [["$NAME", name]]>;
}
