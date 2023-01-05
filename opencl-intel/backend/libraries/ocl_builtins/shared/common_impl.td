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

foreach name = ["clamp"] in {
  defvar builtin = !cast<OclBuiltin>(name);
  def name # _v32v64_impl : OclBuiltinImpl<builtin, [v32f32, v64f32, v32f64, v64f64], 0, clamp_f_code>;
}

foreach name = ["clamp_s"] in {
  defvar builtin = !cast<OclBuiltin>(name);
  defvar types = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64], [1, 2, 3, 4, 8, 16, 32, 64]>.Tout;
  def name # _v32v64_impl : OclBuiltinImpl<builtin, types, 0, clamp_s_code>;
}

foreach name = ["mix", "mix_s", "step_s", "smoothstep_s"] in {
  defvar builtin = !cast<OclBuiltin>(name);
  defvar types = ExpandTypesByVFAndFlatten<[v1f32, v1f64], [1, 2, 3, 4, 8, 16, 32, 64]>.Tout;
  defvar code_impl = !cond(!eq(name, "mix"): mix_code,
                           !eq(name, "mix_s"): mix_s_code,
                           !eq(name, "step_s"): step_s_code,
                           !eq(name, "smoothstep_s"): smoothstep_s_code);
  def name # _v32v64_impl : OclBuiltinImpl<builtin, types, 0, code_impl>;
}

foreach name = ["degrees", "radians"] in {
  defvar builtin = !cast<OclBuiltin>(name);
  defvar f32_types = ExpandTypesByVFAndFlatten<[v1f32], [1, 2, 3, 4, 8, 16, 32, 64]>.Tout;
  defvar f32_code = !cond(!eq(name, "degrees"): degrees_f32_code,
                          !eq(name, "radians"): radians_f32_code);
  def name # _f32_impl : OclBuiltinImpl<builtin, f32_types, 0, f32_code>;

  defvar f64_types = ExpandTypesByVFAndFlatten<[v1f64], [1, 2, 4, 8, 16, 32, 64]>.Tout;
  defvar f64_code = !cond(!eq(name, "degrees"): degrees_f64_code,
                          !eq(name, "radians"): radians_f64_code);
  def name # _f64_impl : OclBuiltinImpl<builtin, f64_types, 0, f64_code>;
}

foreach name = ["smoothstep"] in {
  defvar builtin = !cast<OclBuiltin>(name);
  defvar f32_types = ExpandTypesByVFAndFlatten<[v1f32], [1, 2, 4, 8, 16, 32, 64]>.Tout;
  defvar f32_code = smoothstep_f32_code;
  def name # _f32_impl : OclBuiltinImpl<builtin, f32_types, 0, f32_code>;

  defvar f64_types = ExpandTypesByVFAndFlatten<[v1f64], [1, 2, 4, 8, 16, 32, 64]>.Tout;
  defvar f64_code = smoothstep_f64_code;
  def name # _f64_impl : OclBuiltinImpl<builtin, f64_types, 0, f64_code>;
}

// Expand
foreach name = ["step", "sign"] in {
  defvar builtin = !cast<OclBuiltin>(name);
  defvar num_args = !size(builtin.Ins);
  defvar expand_code = !cond(!eq(num_args, 1): Expand1,
                             !eq(num_args, 2): Expand2);
  def name # _v32v64_impl : OclBuiltinImpl<builtin, [v32f32, v64f32, v32f64, v64f64], 0, expand_code>;
}

// Masked
foreach name = ["clamp", "max", "min", "degrees", "mix", "radians", "step", "smoothstep", "sign"] in {
  defvar builtin = !cast<OclBuiltin>(name # "_mask");
  defvar types = !cond(!or(!eq(name, "clamp"), !eq(name, "max"), !eq(name, "min")): ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64], [1, 2, 3, 4, 8, 16, 32, 64]>.Tout,
                       true: ExpandTypesByVFAndFlatten<[v1f32, v1f64], [1, 2, 3, 4, 8, 16, 32, 64]>.Tout);
  defvar mask_code = !cond(!or(!eq(name, "clamp"), !eq(name, "mix"), !eq(name, "smoothstep")): masked_impl_code_G_G3_M,
                           !or(!eq(name, "max"), !eq(name, "min"), !eq(name, "step")): masked_impl_code_G_G2_M,
                           !or(!eq(name, "degrees"), !eq(name, "radians"), !eq(name, "sign")): masked_impl_code_G_G_M);
  defvar def0 = !cond(!or(!eq(name, "step"), !eq(name, "smoothstep")): "1",
                      true: "0");
  defvar def1 = !cond(!eq(name, "smoothstep"): "2", true: "0");
  def name # _mask_impl : OclBuiltinImpl<builtin, types, 0, mask_code, [["$DEF0", def0], ["$DEF1", def1], ["$DEF2", "0"]]>;
}
