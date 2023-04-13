//
// Copyright (C) 2023 Intel Corporation
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

// To directly support SPIRV builtins requested by SYCL.

// SPV_INTEL_bfloat16_conversion
// https://github.com/intel/llvm/blob/sycl/sycl/doc/design/spirv-extensions/SPV_INTEL_bfloat16_conversion.asciidoc
let Attrs = [CONSTFUNC, OVERLOADABLE], NeedForwardDecl = true in {
  defvar vfs = [1, 2, 3, 4, 8, 16];

  defm spirv_convert_f_to_bf16 : SPIRVBuiltin<"ConvertFToBF16", (outs u16gentype:$ret), (ins gentype:$x), ExpandTypesByVFAndFlatten<[v1f32], vfs>.Tout, /*intel_specific*/true>;
  defm spirv_convert_bf16_to_f : SPIRVBuiltin<"ConvertBF16ToF", (outs f32gentype:$ret), (ins gentype:$x), ExpandTypesByVFAndFlatten<[v1i16, v1u16], vfs>.Tout, /*intel_specific*/true>;
}

// SPV_INTEL_tensor_float32_conversion
// https://github.com/intel/llvm/pull/6990
// Renamed from "ConvertFToTF32" to "RoundFToTF32":
// https://github.com/KhronosGroup/SPIRV-LLVM-Translator/commit/68855f60b26fd630175b4c8696d2a6a7939f21f5
let Attrs = [CONSTFUNC, OVERLOADABLE], NeedForwardDecl = true in {
  defvar vfs = [1, 2, 3, 4, 8, 16];

  defm spirv_round_f_to_tf32 : SPIRVBuiltin<"RoundFToTF32", (outs gentype:$ret), (ins gentype:$x), ExpandTypesByVFAndFlatten<[v1f32], vfs>.Tout, /*intel_specific*/true>;
}

// alias
foreach vf = [1, 2, 3, 4, 8, 16] in {
  defvar vfs = !if(!eq(vf, 1), "", !cast<string>(vf));
  def spirv_convert_f_to_bf16_alias#vf : AliasMap<
    [["spirv_convert_f_to_bf16", "intel_convert_bfloat16" # vfs # "_as_ushort" # vfs]],
    ExpandTypesByVFAndFlatten<[v1f32], [vf]>.Tout
    >;
  def spirv_convert_bf16_to_f_alias#vf : AliasMap<
    [["spirv_convert_bf16_to_f", "intel_convert_as_bfloat16" # vfs # "_float" # vfs]],
    ExpandTypesByVFAndFlatten<[v1i16, v1u16], [vf]>.Tout
    >;
}
