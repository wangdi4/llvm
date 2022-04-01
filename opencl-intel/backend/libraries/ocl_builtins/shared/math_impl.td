// sincos linear strided vector impl
foreach is_native = [true, false] in {
  defvar prefix = !if(is_native, "native_", "");
  foreach is_masked = [true, false] in {
    defvar suffix = "_strided" # !if(is_masked, "_mask", "");
    foreach name = ["sincos_pas", "sincos_las", "sincos_gas", "sincos_gen"] in {
      defvar builtin = !cast<OclBuiltin>(prefix # name # suffix);
      defvar impl_code = !if(is_masked, ret2ptr_strided_mask_code, ret2ptr_strided_code);

      def : OclBuiltinImpl<builtin, builtin.Types, /*IsDeclOnly*/false, impl_code, [["$FUNC", "sincos"]]>;
    } // foreach name

    // sincos_ret2ptr
    defvar builtin = !cast<OclBuiltin>(prefix # "sincos_pas2" # suffix);
    defvar impl_code = !if(is_masked, ret2ptr2_strided_mask_code, ret2ptr2_strided_code);

    def : OclBuiltinImpl<builtin, builtin.Types, /*IsDeclOnly*/false, impl_code, [["$FUNC", "sincos"]]>;
  } // foreach is_masked
} // foreach is_native

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
          ExpandTypesByVFAndFlatten<[v1f64], [1, 2, 4, 8]>.Tout,
      true: ExpandTypesByVFAndFlatten<[v1f32, v1f64], [1, 2, 4, 8]>.Tout);
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
