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
foreach name = ["any", "all"] in {
  defvar builtin = !cast<OclBuiltin>(name);
  def name # _expand_avx512f_impl : OclBuiltinImpl<builtin, [v32f32, v64f32, v32f64, v64f64], 0, Expand1>;
}

foreach name = ["isfinite", "isinf", "isnormal"] in {
  defvar builtin = !cast<OclBuiltin>(name);
  defvar f_code = !cond(!eq(name, "isfinite"): isfinite_code,
                        !eq(name, "isinf"): isinf_code,
                        true : isnormal_code);
  defvar types = ExpandTypesByVFAndFlatten<[v1f16, v1f32, v1f64], [32, 64]>.Tout;
  def name # _impl: OclBuiltinImpl<builtin, types, 0, f_code, [["$Prefix", "h"]]>;
}

// Widened
foreach name = ["isfinite", "isinf", "isnormal"] in {
  defvar builtin = !cast<OclBuiltin>(name # "_v1widen");
  defvar types = ExpandTypesByVFAndFlatten<[v1f16, v1f32, v1f64], [32, 64]>.Tout;
  defvar widen_code = !cond(!eq(name, "isfinite") : isfinite_v1widen_code,
                            !eq(name, "isinf") : isinf_v1widen_code,
                            !eq(name, "isnormal") : isnormal_v1widen_code);
  def name # _v1widen_impl: OclBuiltinImpl<builtin, types, 0, widen_code>;
}

// Masked

foreach name = ["isfinite", "isinf", "isnormal"] in {
  defvar types = ExpandTypesByVFAndFlatten<[v1f16, v1f32, v1f64], [32, 64]>.Tout;
  defvar builtin = !cast<OclBuiltin>(name # "_mask");
  def name # _mask_impl : OclBuiltinImpl<builtin, types, 0, masked_impl_code_I32_G_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;

  defvar builtin_v1widen = !cast<OclBuiltin>(name # "_v1widen_mask");
  def name # _v1widen_mask_impl: OclBuiltinImpl<builtin_v1widen, types, 0, masked_impl_code_I32_G_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;
}
