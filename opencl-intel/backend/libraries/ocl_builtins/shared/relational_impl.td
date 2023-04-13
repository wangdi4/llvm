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

foreach name = ["isfinite", "isinf", "isnormal"] in {
  defvar builtin = !cast<OclBuiltin>(name);
  defvar f_code = !cond(!eq(name, "isfinite"): isfinite_code,
                        !eq(name, "isinf"): isinf_code,
                        true : isnormal_code);
  defvar vfs = [1, 2, 3, 4, 8, 16];
  def name # _f16_impl: OclBuiltinImpl<builtin, ExpandTypesByVFAndFlatten<[v1f16], vfs>.Tout, 0, f_code, [["$Prefix", "h"]]>;
  def name # _f32_impl: OclBuiltinImpl<builtin, ExpandTypesByVFAndFlatten<[v1f32], vfs>.Tout, 0, f_code, [["$Prefix", "f"]]>;
  def name # _f64_impl: OclBuiltinImpl<builtin, ExpandTypesByVFAndFlatten<[v1f64], vfs>.Tout, 0, f_code, [["$Prefix", "d"]]>;
}

// Widened
foreach name = ["isfinite", "isinf", "isnormal"] in {
  defvar builtin = !cast<OclBuiltin>(name # "_v1widen");
  defvar types = ExpandTypesByVFAndFlatten<[v1f16, v1f32, v1f64], [2, 3, 4, 8, 16]>.Tout;
  defvar widen_code = !cond(!eq(name, "isfinite") : isfinite_v1widen_code,
                            !eq(name, "isinf") : isinf_v1widen_code,
                            !eq(name, "isnormal") : isnormal_v1widen_code);
  def name # _v1widen_impl: OclBuiltinImpl<builtin, types, 0, widen_code>;
}

foreach name = ["isfinite", "isinf", "isnormal"] in {
  defvar builtin = !cast<OclBuiltin>(name # "_mask");
  defvar types = ExpandTypesByVFAndFlatten<[v1f16, v1f32, v1f64], [2, 3, 4, 8, 16]>.Tout;
  def name # _mask_impl: OclBuiltinImpl<builtin, types, 0, masked_impl_code_I32_G_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;

  defvar builtin_v1widen = !cast<OclBuiltin>(name # "_v1widen_mask");
  defvar types_v1widen = ExpandTypesByVFAndFlatten<[v1f16, v1f32, v1f64], [4, 8, 16]>.Tout;
  def name # _v1widen_mask_impl: OclBuiltinImpl<builtin_v1widen, types_v1widen, 0, masked_impl_code_I32_G_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;
}
