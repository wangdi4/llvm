// SVML calls
foreach name = ["acos", "acosh", "acospi", "asin", "asinh", "asinpi", "atan",
    "atan2", "atanh", "atanpi", "atan2pi", "cbrt", "cos", "cosh", "cospi",
    "erfc", "erf", "exp", "exp2", "exp10", "expm1", "fmod",
    "fract_pas", "fract_las", "fract_gas", "fract_gen",
    "frexp_pas", "frexp_las", "frexp_gas", "frexp_gen", "hypot",
    "ilogb", "ldexp", "lgamma", "lgamma_r_pas", "lgamma_r_las", "lgamma_r_gas",
    "lgamma_r_gen", "log", "log2", "log10", "log1p", "logb",
    "modf_pas", "modf_las", "modf_gas", "modf_gen", "nextafter",
    "pow", "pown", "powr", "remainder",
    "remquo_pas", "remquo_las", "remquo_gas", "remquo_gen",
    "rootn", "rsqrt", "sin",
    "sincos_pas", "sincos_las", "sincos_gas", "sincos_gen",
    "sinh", "sinpi", "tan", "tanh", "tanpi", "tgamma"] in {
  defvar builtin = !cast<OclBuiltin>(name);
  // Some v8f64 svml builtins has runfail on sse42, so we have to expand them.
  defvar types = !cond(!or(!ne(!find(name, "fract_"), -1), !eq(name, "ilogb")): [v8f64, v16f64],
      true: [v16f32, v8f64, v16f64]);
  defvar num_args = !size(builtin.Ins);
  defvar expand_code = !cond(
      !or(!ne(!find(name, "fract_"), -1), !ne(!find(name, "modf_"), -1), !ne(!find(name, "sincos_"), -1)): Expand_2Ptr,
      !or(!ne(!find(name, "frexp_"), -1), !ne(!find(name, "lgamma_r_"), -1)) : Expand_frexp_lgamma,
      !ne(!find(name, "remquo_"), -1): Expand_remquo,
      !eq(num_args, 1): Expand1,
      !eq(num_args, 2): Expand2,
      !eq(num_args, 3): Expand3);
  def name # _impl : OclBuiltinImpl<builtin, types, 0, expand_code>;
}
