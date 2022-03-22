foreach name = ["acos", "acosh", "acospi", "asin", "asinh", "asinpi", "atan",
    "atan2", "atanh", "atanpi", "atan2pi", "cbrt", "ceil", "cos", "cosh",
    "cospi", "erfc", "erf", "exp", "exp2", "exp10", "expm1", "fdim", "floor",
    "fma", "fmod", "fract_pas", "fract_las", "fract_gas", "fract_gen",
    "fract_pas2", "frexp_pas", "frexp_las", "frexp_gas", "frexp_gen", "hypot",
    "ilogb", "ldexp", "lgamma", "lgamma_r_pas", "lgamma_r_las", "lgamma_r_gas",
    "lgamma_r_gen", "log", "log2", "log10", "log1p", "logb", "modf_pas",
    "modf_las", "modf_gas", "modf_gen", "modf_pas2", "nextafter", "pow", "pown",
    "powr", "remainder", "remquo_pas", "remquo_las", "remquo_gas", "remquo_gen",
    "rint", "rootn", "round", "rsqrt", "sin", "sincos_pas", "sincos_las",
    "sincos_gas", "sincos_gen", "sincos_pas2", "sinh", "sinpi", "sqrt", "tan",
    "tanh", "tanpi", "tgamma", "trunc"] in {
  defvar builtin = !cast<OclBuiltin>(name);

  defvar v32v64_types = [v64f32, v32f64, v64f64];
  defvar v32v64_types_all = ExpandTypesByVFAndFlatten<[v1f32, v1f64], [32, 64]>.Tout;
  defvar types = !cond(
      !or(!ne(!find(name, "fract_"), -1), !eq(name, "ceil"), !eq(name, "fdim"), !eq(name, "floor"), !eq(name, "fma"), !eq(name, "sqrt")): v32v64_types_all,
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
