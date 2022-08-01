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

// Widened
foreach name = ["isfinite", "isinf", "isnan", "isnormal", "isordered", "isunordered", "signbit"] in {
  defvar builtin = !cast<OclBuiltin>(name # "_v1widen");
  defvar types = ExpandTypesByVFAndFlatten<[v1f32, v1f64], [2, 3, 4, 8, 16]>.Tout;
  defvar widen_code = !cond(!eq(name, "isfinite") : isfinite_v1widen_code,
                            !eq(name, "isinf") : isinf_v1widen_code,
                            !eq(name, "isnan") : isnan_v1widen_code,
                            !eq(name, "isnormal") : isnormal_v1widen_code,
                            !eq(name, "isordered") : isordered_v1widen_code,
                            !eq(name, "isunordered") : isunordered_v1widen_code,
                            !eq(name, "signbit") : signbit_v1widen_code);
  def name # _v1widen_impl: OclBuiltinImpl<builtin, types, 0, widen_code>;
}

foreach name = ["isfinite", "isinf", "isnan", "isnormal", "isordered", "isunordered", "signbit"] in {
  defvar builtin = !cast<OclBuiltin>(name # "_mask");
  defvar types = ExpandTypesByVFAndFlatten<[v1f32, v1f64], [2, 3, 4, 8, 16]>.Tout;
  defvar mask_code = !cond(!or(!eq(name, "isordered"), !eq(name, "isunordered")) : masked_impl_code_I32_G2_M,
                           true: masked_impl_code_I32_G_M);
  def name # _mask_impl: OclBuiltinImpl<builtin, types, 0, mask_code, [["$DEF0", "0"], ["$DEF1", "0"]]>;

  defvar builtin_v1widen = !cast<OclBuiltin>(name # "_v1widen_mask");
  defvar types_v1widen = ExpandTypesByVFAndFlatten<[v1f32, v1f64], [4, 8, 16]>.Tout;
  def name # _v1widen_mask_impl: OclBuiltinImpl<builtin_v1widen, types_v1widen, 0, mask_code, [["$DEF0", "0"], ["$DEF1", "0"]]>;
}
