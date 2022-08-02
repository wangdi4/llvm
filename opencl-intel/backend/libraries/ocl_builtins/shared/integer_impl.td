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

defvar all_int_types = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64], [1, 2, 3, 4, 8, 16]>.Tout;
defvar sub_int_types = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32], [1, 2, 3, 4, 8, 16]>.Tout;

// Masked
foreach name = ["abs", "abs_diff", "add_sat", "hadd", "rhadd", "clz", "mad_hi",
                "mad_sat", "mul_hi", "rotate", "sub_sat", "popcount"] in {
  defvar builtin = !cast<OclBuiltin>(name # "_mask");
  defvar mask_code = !cond(!eq(name, "abs") : masked_impl_code_UG_G_M,
      !eq(name, "abs_diff"): masked_impl_code_UG_G2_M,
      !or(!eq(name, "add_sat"), !eq(name, "hadd"), !eq(name, "rhadd"),
          !eq(name, "mul_hi"), !eq(name, "rotate"), !eq(name, "sub_sat")): masked_impl_code_G_G2_M,
      !or(!eq(name, "clz"), !eq(name, "popcount")): masked_impl_code_G_G_M,
      !or(!eq(name, "mad_hi"), !eq(name, "mad_sat")): masked_impl_code_G_G3_M);
  def name # _mask_impl: OclBuiltinImpl<builtin, all_int_types, 0, mask_code, [["$DEF0", "0"], ["$DEF1", "0"], ["$DEF2", "0"]]>;
}

// generated cl code example: upsample
// short16 __attribute__((const)) __attribute__((overloadable)) upsample(char16 hi, uchar16 lo, short16 mask)
// {
//   short16 ret = upsample(hi, lo);
//   return ret & mask;
// }
foreach name = ["upsample"] in {
  defvar builtin = !cast<OclBuiltin>(name # "_mask");
  defvar mask_code = [{
      $ReturnType ret = $Func($Arg0VarName, $Arg1VarName);
      return ret & $Arg2VarName;
    }];
  def name # _mask_impl: OclBuiltinImpl<builtin, sub_int_types, 0, mask_code>;
}
