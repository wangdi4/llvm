defvar v32v64FTypes = ExpandTypesByVFAndFlatten<[v1f32, v1f64], [32, 64]>.Tout;

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
  defvar types = !cond(
      !or(!ne(!find(name, "fract_"), -1), !eq(name, "ilogb")) : [v32f64, v64f64],
      !or(!eq(name, "fma"), !eq(name, "rint")): [v16f64, v32f32, v64f32, v32f64, v64f64],
      !or(!eq(name, "ceil"), !eq(name, "floor"), !eq(name, "sqrt"),
          !eq(name, "native_divide"), !eq(name, "native_recip"),
          !eq(name, "native_rsqrt")): v32v64FTypes,
      true: [v64f32, v32f64, v64f64]);

  defvar builtin = !cast<OclBuiltin>(name);
  defvar num_args = !size(builtin.Ins);
  defvar expand_code = !cond(
      !ne(!find(name, "_pas2"), -1): Expand_2Ptr_NoRet,
      !or(!ne(!find(name, "fract_"), -1), !ne(!find(name, "modf_"), -1), !ne(!find(name, "sincos_"), -1)): Expand_2Ptr,
      !or(!ne(!find(name, "frexp_"), -1), !ne(!find(name, "lgamma_r_"), -1)) : Expand_frexp_lgamma,
      !ne(!find(name, "remquo_"), -1): Expand_remquo,
      !eq(num_args, 1): Expand1,
      !eq(num_args, 2): Expand2,
      !eq(num_args, 3): Expand3);
  def name # _expand_v32v64 : OclBuiltinImpl<builtin, types, 0, expand_code>;
}

// SVML calls
foreach name = ["acos", "acosh", "acospi", "asin", "asinh", "asinpi", "atan",
    "atan2", "atanh", "atanpi", "atan2pi", "cbrt", "cos", "cosh", "cospi",
    "erfc", "erf", "exp", "exp2", "exp10", "expm1", "fmod",
    "fract_pas", "fract_las", "fract_gas", "fract_gen",
    "frexp_pas", "frexp_las", "frexp_gas", "frexp_gen", "hypot",
    "ilogb", "ldexp", "lgamma", "lgamma_r_pas", "lgamma_r_las", "lgamma_r_gas",
    "lgamma_r_gen", "log", "log2", "log10", "log1p", "logb",
    "modf_pas", "modf_las", "modf_gas", "modf_gen", "nextafter",
    "pow", "pown", "powr", "remainder", "remquo_pas", "remquo_las",
    "remquo_gas", "remquo_gen", "rootn", "round", "rsqrt", "sin",
    "sincos_pas", "sincos_las", "sincos_gas", "sincos_gen",
    "sinh", "sinpi", "tan", "tanh", "tanpi", "tgamma"] in {
  defvar builtin = !cast<OclBuiltin>(name);
  defvar types = !cond(!or(!ne(!find(name, "fract_"), -1), !eq(name, "ilogb")): [v16f64],
      true: [v32f32, v16f64]);
  defvar num_args = !size(builtin.Ins);
  defvar svml_code = !cond(!ne(!find(name, "fract_"), -1): fract_svml,
      !ne(!find(name, "frexp_"), -1): frexp_svml,
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
  def name # _avx512f_impl : OclBuiltinImpl<builtin, types, 0, svml_code, [["$NAME", svml_name]]>;
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
  defvar types = [v16f32, v32f32, v8f64, v16f64];
  defvar num_args = !size(builtin.Ins);
  defvar svml_code = !cond(!eq(num_args, 1) : svml_native_code1, true: svml_native_code2);
  def name # _native_impl : OclBuiltinImpl<builtin, types, 0, svml_code, [["$NAME", name]]>;
}

// Masked
foreach name = ["acos", "acosh", "acospi", "asin", "asinh", "asinpi", "atan",
    "atan2", "atanh", "atanpi", "atan2pi", "cbrt", "ceil", "copysign", "cos",
    "cosh", "cospi", "erfc", "erf", "exp", "exp2", "exp10", "expm1", "fabs",
    "fdim", "floor", "fma", "fmax", "fmin", "fmod", "fract_pas", "fract_las",
    "fract_gas", "fract_gen", "fract_pas2", "frexp_pas", "frexp_las",
    "frexp_gas", "frexp_gen", "hypot", "lgamma", "lgamma_r_pas", "lgamma_r_las",
    "lgamma_r_gas", "lgamma_r_gen", "log", "log2", "log10", "log1p", "logb",
    "mad", "maxmag", "minmag", "modf_pas", "modf_las", "modf_gas", "modf_gen",
    "modf_pas2", "nextafter", "pow", "powr", "remainder", "remquo_pas",
    "remquo_las", "remquo_gas", "remquo_gen", "rint", "round", "rsqrt", "sin",
    "sincos_pas", "sincos_las", "sincos_gas", "sincos_gen", "sincos_pas2",
    "sinh", "sinpi", "sqrt", "tan", "tanh", "tanpi", "tgamma", "trunc"] in {
  defvar mask_code = !cond(
      !or(!eq(name, "atan2"), !eq(name, "atan2pi"),
          !eq(name, "copysign"), !eq(name, "fdim"), !eq(name, "fmax"),
          !eq(name, "fmax_common"), !eq(name, "fmin"), !eq(name, "fmin_common"),
          !eq(name, "fmod"), !eq(name, "hypot"), !eq(name, "maxmag"),
          !eq(name, "minmag"), !eq(name, "nextafter"), !eq(name, "pow"),
          !eq(name, "powr"), !eq(name, "remainder")) : masked_impl_code_G_G2_M,
          !or(!eq(name, "fma"), !eq(name, "mad")) : masked_impl_code_G_G3_M,
      !ne(!find(name, "_pas2"), -1) : masked_impl_code_V_G_P_P_M,
      !ne(!find(name, "remquo_"), -1) : masked_impl_code_G_G2_P_M,
      !or(!ne(!find(name, "_pas"), -1), !ne(!find(name, "_las"), -1),
          !ne(!find(name, "_gas"), -1), !ne(!find(name, "_gen"), -1)): masked_impl_code_G_G_P_M,
      true: masked_impl_code_G_G_M);
  defvar def0 = !cond(
      !eq(name, "atanh") : "0.42",
      !eq(name, "erf") : "0.88",
      !or(!eq(name, "rsqrt"), !eq(name, "tgamma")) : "1",
      !or(!eq(name, "acosh"), !eq(name, "asinh"), !eq(name, "lgamma"), !ne(!find(name, "log"), -1), !ne(!find(name, "lgamma_r"), -1)) : "4.2",
      !ne(!find(name, "pow"), -1) : "5",
      true : "0");
  defvar def1 = !cond(
      !ne(!find(name, "pow"), -1) : "5",
      !ne(!find(name, "remquo_"), -1): "42",
      true: "1");
  defvar builtin = !cast<OclBuiltin>(name # "_mask");
  def name # _mask_impl : OclBuiltinImpl<builtin, v32v64FTypes, 0, mask_code, [["$DEF0", def0], ["$DEF1", def1], ["$DEF2", "0"]]>;
}

// Native masked
foreach name = ["acos", "acosh", "acospi", "asin", "asinh", "asinpi", "atan",
    "atan2", "atan2pi", "atanh", "atanpi", "cbrt", "cos", "cosh", "cospi",
    "divide", "erfc", "erf", "exp", "exp2", "exp10", "expm1", "fdim", "fmax",
    "fmin", "fmod", "fract_pas", "fract_las", "fract_gas", "fract_gen",
    "fract_pas2", "hypot", "log", "log2", "log10", "log1p", "logb", "pow",
    "powr", "rcbrt", "recip", "rsqrt", "sin", "sincos_pas", "sincos_las",
    "sincos_gas", "sincos_gen", "sincos_pas2", "sinh", "sinpi", "sqrt",
    "tan", "tanh", "tanpi"] in {
  defvar mask_code = !cond(
      !or(!eq(name, "atan2"), !eq(name, "atan2pi"), !eq(name, "divide"),
          !eq(name, "fdim"), !eq(name, "fmax"), !eq(name, "fmin"),
          !eq(name, "fmod"), !eq(name, "hypot"), !ne(!find(name, "pow"), -1)) : masked_impl_code_G_G2_M,
      !ne(!find(name, "_pas2"), -1) : masked_impl_code_V_G_P_P_M,
      !or(!ne(!find(name, "_pas"), -1), !ne(!find(name, "_las"), -1),
          !ne(!find(name, "_gas"), -1), !ne(!find(name, "_gen"), -1)): masked_impl_code_G_G_P_M,
      true: masked_impl_code_G_G_M);
  defvar def0 = !cond(
      !eq(name, "atanh") : "0.42",
      !eq(name, "erf") : "0.88",
      !or(!eq(name, "divide"), !eq(name, "rcbrt"), !eq(name, "recip"), !eq(name, "rsqrt"), !eq(name, "tgamma")) : "1",
      !or(!eq(name, "acosh"), !eq(name, "asinh"), !eq(name, "lgamma"), !ne(!find(name, "log"), -1)) : "4.2",
      !ne(!find(name, "pow"), -1) : "5",
      true : "0");
  defvar def1 = !cond(
      !or(!eq(name, "fdim"), !eq(name, "fmax"), !eq(name, "fmin"), !eq(name, "hypot")): "0",
      !ne(!find(name, "pow"), -1) : "5",
      true: "1");
  defvar builtin = !cast<OclBuiltin>("native_" # name # "_mask");
  def name # _native_mask_impl : OclBuiltinImpl<builtin, v32v64FTypes, 0, mask_code, [["$DEF0", def0], ["$DEF1", def1], ["$DEF2", "0"]]>;
}
