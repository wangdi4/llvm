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
  defvar types = ExpandTypesByVFAndFlatten<[v1f32, v1f64], [32, 64]>.Tout;
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

// Expand relaxed math
foreach name = [
    "cos_rm", "divide_rm", "exp_rm", "exp2_rm", "exp10_rm",
    "log_rm", "log2_rm", "pow_rm", "sin_rm", "tan_rm",
    "sincos_pas_rm", "sincos_las_rm", "sincos_gas_rm", "sincos_gen_rm"] in {
  defvar builtin = !cast<OclBuiltin>(name);
  defvar types = [v32f32, v64f32];
  defvar num_args = !size(builtin.Ins);
  defvar expand_code = !cond(
      !ne(!find(name, "sincos_"), -1): Expand_2Ptr,
      !eq(num_args, 1): Expand1,
      !eq(num_args, 2): Expand2);
  def name # _expand_v32v64 : OclBuiltinImpl<builtin, types, 0, expand_code>;
}

defvar v32v64FTypes = ExpandTypesByVFAndFlatten<[v1f32, v1f64], [32, 64]>.Tout;

// Masked
foreach name = ["acos", "acosh", "acospi", "asin", "asinh", "asinpi", "atan",
    "atan2", "atanh", "atanpi", "atan2pi", "cbrt", "ceil", "copysign", "cos",
    "cosh", "cospi", "erfc", "erf", "exp", "exp2", "exp10", "expm1",
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
  def name # _rm_mask_impl : OclBuiltinImpl<builtin, v32v64FTypes, 0, mask_code, [["$DEF0", def0], ["$DEF1", def1], ["$DEF2", "0"]]>;
}

defvar v32v64F32Types = ExpandTypesByVFAndFlatten<[v1f32], [32, 64]>.Tout;

// sincos linear strided vector impl
foreach is_masked = [true, false] in {
  foreach is_native = [true, false] in {
    defvar prefix = !if(is_native, "native_", "");
    defvar suffix = "_strided" # !if(is_masked, "_mask", "");
    foreach name = ["sincos_pas", "sincos_las", "sincos_gas", "sincos_gen"] in {
      defvar builtin = !cast<OclBuiltin>(prefix # name # suffix);
      defvar impl_code = !if(is_masked, ret2ptr_strided_mask_code, ret2ptr_strided_code);

      def : OclBuiltinImpl<builtin, v32v64FTypes, /*IsDeclOnly*/false, impl_code, [["$FUNC", "sincos"]]>;
    } // foreach name

    // sincos_ret2ptr
    defvar builtin = !cast<OclBuiltin>(prefix # "sincos_pas2" # suffix);
    defvar impl_code = !if(is_masked, ret2ptr2_strided_mask_code, ret2ptr2_strided_code);

    def : OclBuiltinImpl<builtin, v32v64FTypes, /*IsDeclOnly*/false, impl_code, [["$FUNC", "sincos"]]>;
  } // foreach is_native

  // relaxed
  defvar suffix = "_strided_rm" # !if(is_masked, "_mask", "");
  foreach name = ["sincos_pas", "sincos_las", "sincos_gas", "sincos_gen"] in {
    defvar builtin = !cast<OclBuiltin>(name # suffix);
    defvar impl_code = !if(is_masked, ret2ptr_strided_mask_code, ret2ptr_strided_code);
    def : OclBuiltinImpl<builtin, v32v64F32Types, /*IsDeclOnly*/false, impl_code, [["$FUNC", "sincos_rm"]]>;
  } // foreach name
} // foreach is_masked
