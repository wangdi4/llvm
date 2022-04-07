// To directly support SPIRV builtins requested by SYCL.

// SPV_INTEL_bfloat16_conversion
// https://github.com/intel/llvm/blob/sycl/sycl/doc/design/spirv-extensions/SPV_INTEL_bfloat16_conversion.asciidoc
let Attrs = [CONSTFUNC, OVERLOADABLE], NeedForwardDecl = true in {
  defvar vfs = [1, 2, 3, 4, 8, 16];

  defm spirv_convert_f_to_bf16 : SPIRVBuiltin<"ConvertFToBF16", (outs u16gentype:$ret), (ins gentype:$x), ExpandTypesByVFAndFlatten<[v1f32], vfs>.Tout, /*intel_specific*/true>;
  defm spirv_convert_bf16_to_f : SPIRVBuiltin<"ConvertBF16ToF", (outs f32gentype:$ret), (ins gentype:$x), ExpandTypesByVFAndFlatten<[v1i16, v1u16], vfs>.Tout, /*intel_specific*/true>;
}
