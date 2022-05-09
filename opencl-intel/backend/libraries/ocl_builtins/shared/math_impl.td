defvar f32Types = ExpandTypesByVFAndFlatten<[v1f32], [1, 2, 3, 4, 8, 16]>.Tout;
defvar f32f64Types = ExpandTypesByVFAndFlatten<[v1f32, v1f64], [1, 2, 3, 4, 8, 16]>.Tout;

// sincos linear strided vector impl
foreach is_masked = [true, false] in {
  foreach is_native = [true, false] in {
    defvar prefix = !if(is_native, "native_", "");
    defvar suffix = "_strided" # !if(is_masked, "_mask", "");
    foreach name = ["sincos_pas", "sincos_las", "sincos_gas", "sincos_gen"] in {
      defvar builtin = !cast<OclBuiltin>(prefix # name # suffix);
      defvar impl_code = !if(is_masked, ret2ptr_strided_mask_code, ret2ptr_strided_code);

      def : OclBuiltinImpl<builtin, f32f64Types, /*IsDeclOnly*/false, impl_code, [["$FUNC", "sincos"]]>;
    } // foreach name

    // sincos_ret2ptr
    defvar builtin = !cast<OclBuiltin>(prefix # "sincos_pas2" # suffix);
    defvar impl_code = !if(is_masked, ret2ptr2_strided_mask_code, ret2ptr2_strided_code);

    def : OclBuiltinImpl<builtin, f32f64Types, /*IsDeclOnly*/false, impl_code, [["$FUNC", "sincos"]]>;
  } // foreach is_native

  // relaxed
  defvar suffix = "_strided_rm" # !if(is_masked, "_mask", "");
  foreach name = ["sincos_pas", "sincos_las", "sincos_gas", "sincos_gen"] in {
    defvar builtin = !cast<OclBuiltin>(name # suffix);
    defvar impl_code = !if(is_masked, ret2ptr_strided_mask_code, ret2ptr_strided_code);
    def : OclBuiltinImpl<builtin, f32Types, /*IsDeclOnly*/false, impl_code, [["$FUNC", "sincos_rm"]]>;
  } // foreach name
} // foreach is_masked

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
    "remquo_gas", "remquo_gen", "rootn", "rsqrt", "sin",
    "sincos_pas", "sincos_las", "sincos_gas", "sincos_gen",
    "sinh", "sinpi", "tan", "tanh", "tanpi", "tgamma"] in {
  defvar builtin = !cast<OclBuiltin>(name);
  defvar types = !cond(
      !or(!ne(!find(name, "fract_"), -1), !eq(name, "ilogb"), !eq(name, "rsqrt")):
          ExpandTypesByVFAndFlatten<[v1f64], [1, 2, 4]>.Tout,
      true: [v1f32, v2f32, v4f32, v8f32, v1f64, v2f64, v4f64]);
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
  def name # _impl : OclBuiltinImpl<builtin, types, 0, svml_code, [["$NAME", svml_name]]>;
}

// Relaxed masked
foreach name = ["cos", "divide", "exp", "exp2", "exp10", "log", "log2", "pow",
    "sin", "tan", "sincos_pas", "sincos_las", "sincos_gas", "sincos_gen"] in {
  defvar mask_code = !cond(
      !or(!eq(name, "divide"), !eq(name, "pow")) : masked_impl_code_G_G2_M,
      !or(!ne(!find(name, "_pas"), -1), !ne(!find(name, "_las"), -1),
          !ne(!find(name, "_gas"), -1), !ne(!find(name, "_gen"), -1)): masked_impl_code_G_G_P_M,
      true: masked_impl_code_G_G_M);
  defvar def0 = !cond(
      !ne(!find(name, "log"), -1) : "4.2",
      !ne(!find(name, "pow"), -1) : "5",
      !eq(name, "divide") : "1",
      true : "0");
  defvar def1 = !cond(
      !ne(!find(name, "pow"), -1) : "5",
      true: "1");
  defvar builtin = !cast<OclBuiltin>(name # "_rm_mask");
  defvar types = ExpandTypesByVFAndFlatten<[v1f32], [1, 2, 3, 4, 8, 16]>.Tout;
  def name # _rm_mask_impl : OclBuiltinImpl<builtin, types, 0, mask_code, [["$DEF0", def0], ["$DEF1", def1], ["$DEF2", "0"]]>;
}
