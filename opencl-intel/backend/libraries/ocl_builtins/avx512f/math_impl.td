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
foreach name = ["fma", "rint", "native_rsqrt", "sqrt", "native_sqrt"] in {
  defvar builtin = !cast<OclBuiltin>(name);
  defvar types = [v16f64];
  defvar num_args = !size(builtin.Ins);
  defvar expand_code = !cond(
      !eq(num_args, 1): Expand1,
      !eq(num_args, 3): Expand3);
  def name # _expand_avx512f_impl : OclBuiltinImpl<builtin, types, 0, expand_code>;
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
    "sinpi", "tan", "tanh", "tanpi"] in {
  defvar builtin = !cast<OclBuiltin>("native_" # name);
  defvar types = [v16f32, v32f32, v8f64, v16f64];
  defvar num_args = !size(builtin.Ins);
  defvar svml_code = !cond(!eq(num_args, 1) : svml_native_code1, true: svml_native_code2);
  def name # _native_impl : OclBuiltinImpl<builtin, types, 0, svml_code, [["$NAME", name]]>;
}

// Relaxed SVML calls
foreach name = ["cos", "divide", "exp", "exp2", "exp10", "log", "log2", "pow",
    "sin", "tan", "sincos_pas", "sincos_las", "sincos_gas", "sincos_gen"] in {
  defvar builtin = !cast<OclBuiltin>(name # "_rm");
  defvar num_args = !size(builtin.Ins);
  defvar svml_code = !cond(!ne(!find(name, "sincos_"), -1): svml_rm_code_sincos,
      !eq(name, "divide"): svml_rm_code_divide,
      !eq(num_args, 1) : svml_rm_code1, true: svml_rm_code2);
  def name # _rm_avx512f_impl : OclBuiltinImpl<builtin, [v32f32], 0, svml_code, [["$NAME", name]]>;
}
