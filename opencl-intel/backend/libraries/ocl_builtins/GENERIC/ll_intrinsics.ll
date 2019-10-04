; ModuleID = '<stdin>'
%opencl.sampler_t = type opaque
%opencl.pipe_ro_t = type opaque
%opencl.pipe_wo_t = type opaque
%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage = type { i32, i32, i32 }

define i16 @__ocl_zext_v1i8_v1i16(i8 %x) {
  %1 = zext i8 %x to i16
  ret i16 %1
}

define i16 @__ocl_sext_v1i8_v1i16(i8 %x) {
  %1 = sext i8 %x to i16
  ret i16 %1
}

define i8 @__ocl_trunc_v1i16_v1i8(i16 %x) {
  %1 = trunc i16 %x to i8
  ret i8 %1
}

define i32 @__ocl_zext_v1i8_v1i32(i8 %x) {
  %1 = zext i8 %x to i32
  ret i32 %1
}

define i32 @__ocl_sext_v1i8_v1i32(i8 %x) {
  %1 = sext i8 %x to i32
  ret i32 %1
}

define i8 @__ocl_trunc_v1i32_v1i8(i32 %x) {
  %1 = trunc i32 %x to i8
  ret i8 %1
}

define i64 @__ocl_zext_v1i8_v1i64(i8 %x) {
  %1 = zext i8 %x to i64
  ret i64 %1
}

define i64 @__ocl_sext_v1i8_v1i64(i8 %x) {
  %1 = sext i8 %x to i64
  ret i64 %1
}

define i8 @__ocl_trunc_v1i64_v1i8(i64 %x) {
  %1 = trunc i64 %x to i8
  ret i8 %1
}

define i32 @__ocl_zext_v1i16_v1i32(i16 %x) {
  %1 = zext i16 %x to i32
  ret i32 %1
}

define i32 @__ocl_sext_v1i16_v1i32(i16 %x) {
  %1 = sext i16 %x to i32
  ret i32 %1
}

define i16 @__ocl_trunc_v1i32_v1i16(i32 %x) {
  %1 = trunc i32 %x to i16
  ret i16 %1
}

define i64 @__ocl_zext_v1i16_v1i64(i16 %x) {
  %1 = zext i16 %x to i64
  ret i64 %1
}

define i64 @__ocl_sext_v1i16_v1i64(i16 %x) {
  %1 = sext i16 %x to i64
  ret i64 %1
}

define i16 @__ocl_trunc_v1i64_v1i16(i64 %x) {
  %1 = trunc i64 %x to i16
  ret i16 %1
}

define i64 @__ocl_zext_v1i32_v1i64(i32 %x) {
  %1 = zext i32 %x to i64
  ret i64 %1
}

define i64 @__ocl_sext_v1i32_v1i64(i32 %x) {
  %1 = sext i32 %x to i64
  ret i64 %1
}

define i32 @__ocl_trunc_v1i64_v1i32(i64 %x) {
  %1 = trunc i64 %x to i32
  ret i32 %1
}

define <2 x i16> @__ocl_zext_v2i8_v2i16(<2 x i8> %x) {
  %1 = zext <2 x i8> %x to <2 x i16>
  ret <2 x i16> %1
}

define <2 x i16> @__ocl_sext_v2i8_v2i16(<2 x i8> %x) {
  %1 = sext <2 x i8> %x to <2 x i16>
  ret <2 x i16> %1
}

define <2 x i8> @__ocl_trunc_v2i16_v2i8(<2 x i16> %x) {
  %1 = trunc <2 x i16> %x to <2 x i8>
  ret <2 x i8> %1
}

define <2 x i32> @__ocl_zext_v2i8_v2i32(<2 x i8> %x) {
  %1 = zext <2 x i8> %x to <2 x i32>
  ret <2 x i32> %1
}

define <2 x i32> @__ocl_sext_v2i8_v2i32(<2 x i8> %x) {
  %1 = sext <2 x i8> %x to <2 x i32>
  ret <2 x i32> %1
}

define <2 x i8> @__ocl_trunc_v2i32_v2i8(<2 x i32> %x) {
  %1 = trunc <2 x i32> %x to <2 x i8>
  ret <2 x i8> %1
}

define <2 x i64> @__ocl_zext_v2i8_v2i64(<2 x i8> %x) {
  %1 = zext <2 x i8> %x to <2 x i64>
  ret <2 x i64> %1
}

define <2 x i64> @__ocl_sext_v2i8_v2i64(<2 x i8> %x) {
  %1 = sext <2 x i8> %x to <2 x i64>
  ret <2 x i64> %1
}

define <2 x i8> @__ocl_trunc_v2i64_v2i8(<2 x i64> %x) {
  %1 = trunc <2 x i64> %x to <2 x i8>
  ret <2 x i8> %1
}

define <2 x i32> @__ocl_zext_v2i16_v2i32(<2 x i16> %x) {
  %1 = zext <2 x i16> %x to <2 x i32>
  ret <2 x i32> %1
}

define <2 x i32> @__ocl_sext_v2i16_v2i32(<2 x i16> %x) {
  %1 = sext <2 x i16> %x to <2 x i32>
  ret <2 x i32> %1
}

define <2 x i16> @__ocl_trunc_v2i32_v2i16(<2 x i32> %x) {
  %1 = trunc <2 x i32> %x to <2 x i16>
  ret <2 x i16> %1
}

define <2 x i64> @__ocl_zext_v2i16_v2i64(<2 x i16> %x) {
  %1 = zext <2 x i16> %x to <2 x i64>
  ret <2 x i64> %1
}

define <2 x i64> @__ocl_sext_v2i16_v2i64(<2 x i16> %x) {
  %1 = sext <2 x i16> %x to <2 x i64>
  ret <2 x i64> %1
}

define <2 x i16> @__ocl_trunc_v2i64_v2i16(<2 x i64> %x) {
  %1 = trunc <2 x i64> %x to <2 x i16>
  ret <2 x i16> %1
}

define <2 x i64> @__ocl_zext_v2i32_v2i64(<2 x i32> %x) {
  %1 = zext <2 x i32> %x to <2 x i64>
  ret <2 x i64> %1
}

define <2 x i64> @__ocl_sext_v2i32_v2i64(<2 x i32> %x) {
  %1 = sext <2 x i32> %x to <2 x i64>
  ret <2 x i64> %1
}

define <2 x i32> @__ocl_trunc_v2i64_v2i32(<2 x i64> %x) {
  %1 = trunc <2 x i64> %x to <2 x i32>
  ret <2 x i32> %1
}

define <3 x i16> @__ocl_zext_v3i8_v3i16(<3 x i8> %x) {
  %1 = zext <3 x i8> %x to <3 x i16>
  ret <3 x i16> %1
}

define <3 x i16> @__ocl_sext_v3i8_v3i16(<3 x i8> %x) {
  %1 = sext <3 x i8> %x to <3 x i16>
  ret <3 x i16> %1
}

define <3 x i8> @__ocl_trunc_v3i16_v3i8(<3 x i16> %x) {
  %1 = trunc <3 x i16> %x to <3 x i8>
  ret <3 x i8> %1
}

define <3 x i32> @__ocl_zext_v3i8_v3i32(<3 x i8> %x) {
  %1 = zext <3 x i8> %x to <3 x i32>
  ret <3 x i32> %1
}

define <3 x i32> @__ocl_sext_v3i8_v3i32(<3 x i8> %x) {
  %1 = sext <3 x i8> %x to <3 x i32>
  ret <3 x i32> %1
}

define <3 x i8> @__ocl_trunc_v3i32_v3i8(<3 x i32> %x) {
  %1 = trunc <3 x i32> %x to <3 x i8>
  ret <3 x i8> %1
}

define <3 x i64> @__ocl_zext_v3i8_v3i64(<3 x i8> %x) {
  %1 = zext <3 x i8> %x to <3 x i64>
  ret <3 x i64> %1
}

define <3 x i64> @__ocl_sext_v3i8_v3i64(<3 x i8> %x) {
  %1 = sext <3 x i8> %x to <3 x i64>
  ret <3 x i64> %1
}

define <3 x i8> @__ocl_trunc_v3i64_v3i8(<3 x i64> %x) {
  %1 = trunc <3 x i64> %x to <3 x i8>
  ret <3 x i8> %1
}

define <3 x i32> @__ocl_zext_v3i16_v3i32(<3 x i16> %x) {
  %1 = zext <3 x i16> %x to <3 x i32>
  ret <3 x i32> %1
}

define <3 x i32> @__ocl_sext_v3i16_v3i32(<3 x i16> %x) {
  %1 = sext <3 x i16> %x to <3 x i32>
  ret <3 x i32> %1
}

define <3 x i16> @__ocl_trunc_v3i32_v3i16(<3 x i32> %x) {
  %1 = trunc <3 x i32> %x to <3 x i16>
  ret <3 x i16> %1
}

define <3 x i64> @__ocl_zext_v3i16_v3i64(<3 x i16> %x) {
  %1 = zext <3 x i16> %x to <3 x i64>
  ret <3 x i64> %1
}

define <3 x i64> @__ocl_sext_v3i16_v3i64(<3 x i16> %x) {
  %1 = sext <3 x i16> %x to <3 x i64>
  ret <3 x i64> %1
}

define <3 x i16> @__ocl_trunc_v3i64_v3i16(<3 x i64> %x) {
  %1 = trunc <3 x i64> %x to <3 x i16>
  ret <3 x i16> %1
}

define <3 x i64> @__ocl_zext_v3i32_v3i64(<3 x i32> %x) {
  %1 = zext <3 x i32> %x to <3 x i64>
  ret <3 x i64> %1
}

define <3 x i64> @__ocl_sext_v3i32_v3i64(<3 x i32> %x) {
  %1 = sext <3 x i32> %x to <3 x i64>
  ret <3 x i64> %1
}

define <3 x i32> @__ocl_trunc_v3i64_v3i32(<3 x i64> %x) {
  %1 = trunc <3 x i64> %x to <3 x i32>
  ret <3 x i32> %1
}

define <4 x i16> @__ocl_zext_v4i8_v4i16(<4 x i8> %x) {
  %1 = zext <4 x i8> %x to <4 x i16>
  ret <4 x i16> %1
}

define <4 x i16> @__ocl_sext_v4i8_v4i16(<4 x i8> %x) {
  %1 = sext <4 x i8> %x to <4 x i16>
  ret <4 x i16> %1
}

define <4 x i8> @__ocl_trunc_v4i16_v4i8(<4 x i16> %x) {
  %1 = trunc <4 x i16> %x to <4 x i8>
  ret <4 x i8> %1
}

define <4 x i32> @__ocl_zext_v4i8_v4i32(<4 x i8> %x) {
  %1 = zext <4 x i8> %x to <4 x i32>
  ret <4 x i32> %1
}

define <4 x i32> @__ocl_sext_v4i8_v4i32(<4 x i8> %x) {
  %1 = sext <4 x i8> %x to <4 x i32>
  ret <4 x i32> %1
}

define <4 x i8> @__ocl_trunc_v4i32_v4i8(<4 x i32> %x) {
  %1 = trunc <4 x i32> %x to <4 x i8>
  ret <4 x i8> %1
}

define <4 x i64> @__ocl_zext_v4i8_v4i64(<4 x i8> %x) {
  %1 = zext <4 x i8> %x to <4 x i64>
  ret <4 x i64> %1
}

define <4 x i64> @__ocl_sext_v4i8_v4i64(<4 x i8> %x) {
  %1 = sext <4 x i8> %x to <4 x i64>
  ret <4 x i64> %1
}

define <4 x i8> @__ocl_trunc_v4i64_v4i8(<4 x i64> %x) {
  %1 = trunc <4 x i64> %x to <4 x i8>
  ret <4 x i8> %1
}

define <4 x i32> @__ocl_zext_v4i16_v4i32(<4 x i16> %x) {
  %1 = zext <4 x i16> %x to <4 x i32>
  ret <4 x i32> %1
}

define <4 x i32> @__ocl_sext_v4i16_v4i32(<4 x i16> %x) {
  %1 = sext <4 x i16> %x to <4 x i32>
  ret <4 x i32> %1
}

define <4 x i16> @__ocl_trunc_v4i32_v4i16(<4 x i32> %x) {
  %1 = trunc <4 x i32> %x to <4 x i16>
  ret <4 x i16> %1
}

define <4 x i64> @__ocl_zext_v4i16_v4i64(<4 x i16> %x) {
  %1 = zext <4 x i16> %x to <4 x i64>
  ret <4 x i64> %1
}

define <4 x i64> @__ocl_sext_v4i16_v4i64(<4 x i16> %x) {
  %1 = sext <4 x i16> %x to <4 x i64>
  ret <4 x i64> %1
}

define <4 x i16> @__ocl_trunc_v4i64_v4i16(<4 x i64> %x) {
  %1 = trunc <4 x i64> %x to <4 x i16>
  ret <4 x i16> %1
}

define <4 x i64> @__ocl_zext_v4i32_v4i64(<4 x i32> %x) {
  %1 = zext <4 x i32> %x to <4 x i64>
  ret <4 x i64> %1
}

define <4 x i64> @__ocl_sext_v4i32_v4i64(<4 x i32> %x) {
  %1 = sext <4 x i32> %x to <4 x i64>
  ret <4 x i64> %1
}

define <4 x i32> @__ocl_trunc_v4i64_v4i32(<4 x i64> %x) {
  %1 = trunc <4 x i64> %x to <4 x i32>
  ret <4 x i32> %1
}

define <8 x i16> @__ocl_zext_v8i8_v8i16(<8 x i8> %x) {
  %1 = zext <8 x i8> %x to <8 x i16>
  ret <8 x i16> %1
}

define <8 x i16> @__ocl_sext_v8i8_v8i16(<8 x i8> %x) {
  %1 = sext <8 x i8> %x to <8 x i16>
  ret <8 x i16> %1
}

define <8 x i8> @__ocl_trunc_v8i16_v8i8(<8 x i16> %x) {
  %1 = trunc <8 x i16> %x to <8 x i8>
  ret <8 x i8> %1
}

define <8 x i32> @__ocl_zext_v8i8_v8i32(<8 x i8> %x) {
  %1 = zext <8 x i8> %x to <8 x i32>
  ret <8 x i32> %1
}

define <8 x i32> @__ocl_sext_v8i8_v8i32(<8 x i8> %x) {
  %1 = sext <8 x i8> %x to <8 x i32>
  ret <8 x i32> %1
}

define <8 x i8> @__ocl_trunc_v8i32_v8i8(<8 x i32> %x) {
  %1 = trunc <8 x i32> %x to <8 x i8>
  ret <8 x i8> %1
}

define <8 x i64> @__ocl_zext_v8i8_v8i64(<8 x i8> %x) {
  %1 = zext <8 x i8> %x to <8 x i64>
  ret <8 x i64> %1
}

define <8 x i64> @__ocl_sext_v8i8_v8i64(<8 x i8> %x) {
  %1 = sext <8 x i8> %x to <8 x i64>
  ret <8 x i64> %1
}

define <8 x i8> @__ocl_trunc_v8i64_v8i8(<8 x i64> %x) {
  %1 = trunc <8 x i64> %x to <8 x i8>
  ret <8 x i8> %1
}

define <8 x i32> @__ocl_zext_v8i16_v8i32(<8 x i16> %x) {
  %1 = zext <8 x i16> %x to <8 x i32>
  ret <8 x i32> %1
}

define <8 x i32> @__ocl_sext_v8i16_v8i32(<8 x i16> %x) {
  %1 = sext <8 x i16> %x to <8 x i32>
  ret <8 x i32> %1
}

define <8 x i16> @__ocl_trunc_v8i32_v8i16(<8 x i32> %x) {
  %1 = trunc <8 x i32> %x to <8 x i16>
  ret <8 x i16> %1
}

define <8 x i64> @__ocl_zext_v8i16_v8i64(<8 x i16> %x) {
  %1 = zext <8 x i16> %x to <8 x i64>
  ret <8 x i64> %1
}

define <8 x i64> @__ocl_sext_v8i16_v8i64(<8 x i16> %x) {
  %1 = sext <8 x i16> %x to <8 x i64>
  ret <8 x i64> %1
}

define <8 x i16> @__ocl_trunc_v8i64_v8i16(<8 x i64> %x) {
  %1 = trunc <8 x i64> %x to <8 x i16>
  ret <8 x i16> %1
}

define <8 x i64> @__ocl_zext_v8i32_v8i64(<8 x i32> %x) {
  %1 = zext <8 x i32> %x to <8 x i64>
  ret <8 x i64> %1
}

define <8 x i64> @__ocl_sext_v8i32_v8i64(<8 x i32> %x) {
  %1 = sext <8 x i32> %x to <8 x i64>
  ret <8 x i64> %1
}

define <8 x i32> @__ocl_trunc_v8i64_v8i32(<8 x i64> %x) {
  %1 = trunc <8 x i64> %x to <8 x i32>
  ret <8 x i32> %1
}

define <16 x i16> @__ocl_zext_v16i8_v16i16(<16 x i8> %x) {
  %1 = zext <16 x i8> %x to <16 x i16>
  ret <16 x i16> %1
}

define <16 x i16> @__ocl_sext_v16i8_v16i16(<16 x i8> %x) {
  %1 = sext <16 x i8> %x to <16 x i16>
  ret <16 x i16> %1
}

define <16 x i8> @__ocl_trunc_v16i16_v16i8(<16 x i16> %x) {
  %1 = trunc <16 x i16> %x to <16 x i8>
  ret <16 x i8> %1
}

define <16 x i32> @__ocl_zext_v16i8_v16i32(<16 x i8> %x) {
  %1 = zext <16 x i8> %x to <16 x i32>
  ret <16 x i32> %1
}

define <16 x i32> @__ocl_sext_v16i8_v16i32(<16 x i8> %x) {
  %1 = sext <16 x i8> %x to <16 x i32>
  ret <16 x i32> %1
}

define <16 x i8> @__ocl_trunc_v16i32_v16i8(<16 x i32> %x) {
  %1 = trunc <16 x i32> %x to <16 x i8>
  ret <16 x i8> %1
}

define <16 x i64> @__ocl_zext_v16i8_v16i64(<16 x i8> %x) {
  %1 = zext <16 x i8> %x to <16 x i64>
  ret <16 x i64> %1
}

define <16 x i64> @__ocl_sext_v16i8_v16i64(<16 x i8> %x) {
  %1 = sext <16 x i8> %x to <16 x i64>
  ret <16 x i64> %1
}

define <16 x i8> @__ocl_trunc_v16i64_v16i8(<16 x i64> %x) {
  %1 = trunc <16 x i64> %x to <16 x i8>
  ret <16 x i8> %1
}

define <16 x i32> @__ocl_zext_v16i16_v16i32(<16 x i16> %x) {
  %1 = zext <16 x i16> %x to <16 x i32>
  ret <16 x i32> %1
}

define <16 x i32> @__ocl_sext_v16i16_v16i32(<16 x i16> %x) {
  %1 = sext <16 x i16> %x to <16 x i32>
  ret <16 x i32> %1
}

define <16 x i16> @__ocl_trunc_v16i32_v16i16(<16 x i32> %x) {
  %1 = trunc <16 x i32> %x to <16 x i16>
  ret <16 x i16> %1
}

define <16 x i64> @__ocl_zext_v16i16_v16i64(<16 x i16> %x) {
  %1 = zext <16 x i16> %x to <16 x i64>
  ret <16 x i64> %1
}

define <16 x i64> @__ocl_sext_v16i16_v16i64(<16 x i16> %x) {
  %1 = sext <16 x i16> %x to <16 x i64>
  ret <16 x i64> %1
}

define <16 x i16> @__ocl_trunc_v16i64_v16i16(<16 x i64> %x) {
  %1 = trunc <16 x i64> %x to <16 x i16>
  ret <16 x i16> %1
}

define <16 x i64> @__ocl_zext_v16i32_v16i64(<16 x i32> %x) {
  %1 = zext <16 x i32> %x to <16 x i64>
  ret <16 x i64> %1
}

define <16 x i64> @__ocl_sext_v16i32_v16i64(<16 x i32> %x) {
  %1 = sext <16 x i32> %x to <16 x i64>
  ret <16 x i64> %1
}

define <16 x i32> @__ocl_trunc_v16i64_v16i32(<16 x i64> %x) {
  %1 = trunc <16 x i64> %x to <16 x i32>
  ret <16 x i32> %1
}

define float @__ocl_trunc_double_float(double %x) {
  %1 = fptrunc double %x to float
  ret float %1
}

define <2 x float> @__ocl_trunc_double2_float2(<2 x double> %x) {
  %1 = fptrunc <2 x double> %x to <2 x float>
  ret <2 x float> %1
}

define <3 x float> @__ocl_trunc_double3_float3(<3 x double> %x) {
  %1 = fptrunc <3 x double> %x to <3 x float>
  ret <3 x float> %1
}

define <4 x float> @__ocl_trunc_double4_float4(<4 x double> %x) {
  %1 = fptrunc <4 x double> %x to <4 x float>
  ret <4 x float> %1
}

define <8 x float> @__ocl_trunc_double8_float8(<8 x double> %x) {
  %1 = fptrunc <8 x double> %x to <8 x float>
  ret <8 x float> %1
}

define <16 x float> @__ocl_trunc_double16_float16(<16 x double> %x) {
  %1 = fptrunc <16 x double> %x to <16 x float>
  ret <16 x float> %1
}

define <8 x i64> @__ocl_select_v8i64(<8 x i64> %x, <8 x i64> %y, <8 x i64> %c) {
  %and = and <8 x i64> %c, <i64 -9223372036854775808, i64 -9223372036854775808, i64 -9223372036854775808, i64 -9223372036854775808, i64 -9223372036854775808, i64 -9223372036854775808, i64 -9223372036854775808, i64 -9223372036854775808>
  %mask = icmp eq <8 x i64> %and, <i64 -9223372036854775808, i64 -9223372036854775808, i64 -9223372036854775808, i64 -9223372036854775808, i64 -9223372036854775808, i64 -9223372036854775808, i64 -9223372036854775808, i64 -9223372036854775808>
  %selected = select <8 x i1> %mask, <8 x i64> %y, <8 x i64> %x
  ret <8 x i64> %selected
}

define <16 x i32> @__ocl_select_v16i32(<16 x i32> %x, <16 x i32> %y, <16 x i32> %c) {
  %and = and <16 x i32> %c, <i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648>
  %mask = icmp eq <16 x i32> %and, <i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648>
  %selected = select <16 x i1> %mask, <16 x i32> %y, <16 x i32> %x
  ret <16 x i32> %selected
}

define %opencl.sampler_t addrspace(2)* @__translate_sampler_initializer(i32 %initVal) {
  %astype = inttoptr i32 %initVal to %opencl.sampler_t addrspace(2)*
  ret %opencl.sampler_t addrspace(2)* %astype
}

declare i8 @llvm.ctlz.i8(i8, i1) nounwind readonly
declare i16 @llvm.ctlz.i16(i16, i1) nounwind readonly
declare i32 @llvm.ctlz.i32(i32, i1) nounwind readonly
declare i64 @llvm.ctlz.i64(i64, i1) nounwind readonly

define i8 @__ocl_helper_clz_v1u8(i8 %x) {
  %1 = tail call i8 @llvm.ctlz.i8(i8 %x, i1 false)
  ret i8 %1
}

define i16 @__ocl_helper_clz_v1u16(i16 %x) {
  %1 = tail call i16 @llvm.ctlz.i16(i16 %x, i1 false)
  ret i16 %1
}

define i32 @__ocl_helper_clz_v1u32(i32 %x) {
  %1 = tail call i32 @llvm.ctlz.i32(i32 %x, i1 false)
  ret i32 %1
}

define i64 @__ocl_helper_clz_v1u64(i64 %x) {
  %1 = tail call i64 @llvm.ctlz.i64(i64 %x, i1 false)
  ret i64 %1
}

declare <2 x i8> @llvm.ctlz.v2i8(<2 x i8>, i1) nounwind readonly
declare <2 x i16> @llvm.ctlz.v2i16(<2 x i16>, i1) nounwind readonly
declare <2 x i32> @llvm.ctlz.v2i32(<2 x i32>, i1) nounwind readonly
declare <2 x i64> @llvm.ctlz.v2i64(<2 x i64>, i1) nounwind readonly

define <2 x i8> @__ocl_helper_clz_v2u8(<2 x i8> %x) {
  %1 = tail call <2 x i8> @llvm.ctlz.v2i8(<2 x i8> %x, i1 false)
  ret <2 x i8> %1
}

define <2 x i16> @__ocl_helper_clz_v2u16(<2 x i16> %x) {
  %1 = tail call <2 x i16> @llvm.ctlz.v2i16(<2 x i16> %x, i1 false)
  ret <2 x i16> %1
}

define <2 x i32> @__ocl_helper_clz_v2u32(<2 x i32> %x) {
  %1 = tail call <2 x i32> @llvm.ctlz.v2i32(<2 x i32> %x, i1 false)
  ret <2 x i32> %1
}

define <2 x i64> @__ocl_helper_clz_v2u64(<2 x i64> %x) {
  %1 = tail call <2 x i64> @llvm.ctlz.v2i64(<2 x i64> %x, i1 false)
  ret <2 x i64> %1
}

declare <4 x i8> @llvm.ctlz.v4i8(<4 x i8>, i1) nounwind readonly
declare <4 x i16> @llvm.ctlz.v4i16(<4 x i16>, i1) nounwind readonly
declare <4 x i32> @llvm.ctlz.v4i32(<4 x i32>, i1) nounwind readonly
declare <4 x i64> @llvm.ctlz.v4i64(<4 x i64>, i1) nounwind readonly

define <4 x i8> @__ocl_helper_clz_v4u8(<4 x i8> %x) {
  %1 = tail call <4 x i8> @llvm.ctlz.v4i8(<4 x i8> %x, i1 false)
  ret <4 x i8> %1
}

define <4 x i16> @__ocl_helper_clz_v4u16(<4 x i16> %x) {
  %1 = tail call <4 x i16> @llvm.ctlz.v4i16(<4 x i16> %x, i1 false)
  ret <4 x i16> %1
}

define <4 x i32> @__ocl_helper_clz_v4u32(<4 x i32> %x) {
  %1 = tail call <4 x i32> @llvm.ctlz.v4i32(<4 x i32> %x, i1 false)
  ret <4 x i32> %1
}

define <4 x i64> @__ocl_helper_clz_v4u64(<4 x i64> %x) {
  %1 = tail call <4 x i64> @llvm.ctlz.v4i64(<4 x i64> %x, i1 false)
  ret <4 x i64> %1
}

declare <8 x i8> @llvm.ctlz.v8i8(<8 x i8>, i1) nounwind readonly
declare <8 x i16> @llvm.ctlz.v8i16(<8 x i16>, i1) nounwind readonly
declare <8 x i32> @llvm.ctlz.v8i32(<8 x i32>, i1) nounwind readonly
declare <8 x i64> @llvm.ctlz.v8i64(<8 x i64>, i1) nounwind readonly

define <8 x i8> @__ocl_helper_clz_v8u8(<8 x i8> %x) {
  %1 = tail call <8 x i8> @llvm.ctlz.v8i8(<8 x i8> %x, i1 false)
  ret <8 x i8> %1
}

define <8 x i16> @__ocl_helper_clz_v8u16(<8 x i16> %x) {
  %1 = tail call <8 x i16> @llvm.ctlz.v8i16(<8 x i16> %x, i1 false)
  ret <8 x i16> %1
}

define <8 x i32> @__ocl_helper_clz_v8u32(<8 x i32> %x) {
  %1 = tail call <8 x i32> @llvm.ctlz.v8i32(<8 x i32> %x, i1 false)
  ret <8 x i32> %1
}

define <8 x i64> @__ocl_helper_clz_v8u64(<8 x i64> %x) {
  %1 = tail call <8 x i64> @llvm.ctlz.v8i64(<8 x i64> %x, i1 false)
  ret <8 x i64> %1
}

declare <16 x i8> @llvm.ctlz.v16i8(<16 x i8>, i1) nounwind readonly
declare <16 x i16> @llvm.ctlz.v16i16(<16 x i16>, i1) nounwind readonly
declare <16 x i32> @llvm.ctlz.v16i32(<16 x i32>, i1) nounwind readonly
declare <16 x i64> @llvm.ctlz.v16i64(<16 x i64>, i1) nounwind readonly

define <16 x i8> @__ocl_helper_clz_v16u8(<16 x i8> %x) {
  %1 = tail call <16 x i8> @llvm.ctlz.v16i8(<16 x i8> %x, i1 false)
  ret <16 x i8> %1
}

define <16 x i16> @__ocl_helper_clz_v16u16(<16 x i16> %x) {
  %1 = tail call <16 x i16> @llvm.ctlz.v16i16(<16 x i16> %x, i1 false)
  ret <16 x i16> %1
}

define <16 x i32> @__ocl_helper_clz_v16u32(<16 x i32> %x) {
  %1 = tail call <16 x i32> @llvm.ctlz.v16i32(<16 x i32> %x, i1 false)
  ret <16 x i32> %1
}

define <16 x i64> @__ocl_helper_clz_v16u64(<16 x i64> %x) {
  %1 = tail call <16 x i64> @llvm.ctlz.v16i64(<16 x i64> %x, i1 false)
  ret <16 x i64> %1
}

declare i8 @llvm.cttz.i8(i8, i1) nounwind readonly
declare i16 @llvm.cttz.i16(i16, i1) nounwind readonly
declare i32 @llvm.cttz.i32(i32, i1) nounwind readonly
declare i64 @llvm.cttz.i64(i64, i1) nounwind readonly

define i8 @__ocl_helper_ctz_v1u8(i8 %x) {
  %1 = tail call i8 @llvm.cttz.i8(i8 %x, i1 false)
  ret i8 %1
}

define i16 @__ocl_helper_ctz_v1u16(i16 %x) {
  %1 = tail call i16 @llvm.cttz.i16(i16 %x, i1 false)
  ret i16 %1
}

define i32 @__ocl_helper_ctz_v1u32(i32 %x) {
  %1 = tail call i32 @llvm.cttz.i32(i32 %x, i1 false)
  ret i32 %1
}

define i64 @__ocl_helper_ctz_v1u64(i64 %x) {
  %1 = tail call i64 @llvm.cttz.i64(i64 %x, i1 false)
  ret i64 %1
}

declare <2 x i8> @llvm.cttz.v2i8(<2 x i8>, i1) nounwind readonly
declare <2 x i16> @llvm.cttz.v2i16(<2 x i16>, i1) nounwind readonly
declare <2 x i32> @llvm.cttz.v2i32(<2 x i32>, i1) nounwind readonly
declare <2 x i64> @llvm.cttz.v2i64(<2 x i64>, i1) nounwind readonly

define <2 x i8> @__ocl_helper_ctz_v2u8(<2 x i8> %x) {
  %1 = tail call <2 x i8> @llvm.cttz.v2i8(<2 x i8> %x, i1 false)
  ret <2 x i8> %1
}

define <2 x i16> @__ocl_helper_ctz_v2u16(<2 x i16> %x) {
  %1 = tail call <2 x i16> @llvm.cttz.v2i16(<2 x i16> %x, i1 false)
  ret <2 x i16> %1
}

define <2 x i32> @__ocl_helper_ctz_v2u32(<2 x i32> %x) {
  %1 = tail call <2 x i32> @llvm.cttz.v2i32(<2 x i32> %x, i1 false)
  ret <2 x i32> %1
}

define <2 x i64> @__ocl_helper_ctz_v2u64(<2 x i64> %x) {
  %1 = tail call <2 x i64> @llvm.cttz.v2i64(<2 x i64> %x, i1 false)
  ret <2 x i64> %1
}

declare <4 x i8> @llvm.cttz.v4i8(<4 x i8>, i1) nounwind readonly
declare <4 x i16> @llvm.cttz.v4i16(<4 x i16>, i1) nounwind readonly
declare <4 x i32> @llvm.cttz.v4i32(<4 x i32>, i1) nounwind readonly
declare <4 x i64> @llvm.cttz.v4i64(<4 x i64>, i1) nounwind readonly

define <4 x i8> @__ocl_helper_ctz_v4u8(<4 x i8> %x) {
  %1 = tail call <4 x i8> @llvm.cttz.v4i8(<4 x i8> %x, i1 false)
  ret <4 x i8> %1
}

define <4 x i16> @__ocl_helper_ctz_v4u16(<4 x i16> %x) {
  %1 = tail call <4 x i16> @llvm.cttz.v4i16(<4 x i16> %x, i1 false)
  ret <4 x i16> %1
}

define <4 x i32> @__ocl_helper_ctz_v4u32(<4 x i32> %x) {
  %1 = tail call <4 x i32> @llvm.cttz.v4i32(<4 x i32> %x, i1 false)
  ret <4 x i32> %1
}

define <4 x i64> @__ocl_helper_ctz_v4u64(<4 x i64> %x) {
  %1 = tail call <4 x i64> @llvm.cttz.v4i64(<4 x i64> %x, i1 false)
  ret <4 x i64> %1
}

declare <8 x i8> @llvm.cttz.v8i8(<8 x i8>, i1) nounwind readonly
declare <8 x i16> @llvm.cttz.v8i16(<8 x i16>, i1) nounwind readonly
declare <8 x i32> @llvm.cttz.v8i32(<8 x i32>, i1) nounwind readonly
declare <8 x i64> @llvm.cttz.v8i64(<8 x i64>, i1) nounwind readonly

define <8 x i8> @__ocl_helper_ctz_v8u8(<8 x i8> %x) {
  %1 = tail call <8 x i8> @llvm.cttz.v8i8(<8 x i8> %x, i1 false)
  ret <8 x i8> %1
}

define <8 x i16> @__ocl_helper_ctz_v8u16(<8 x i16> %x) {
  %1 = tail call <8 x i16> @llvm.cttz.v8i16(<8 x i16> %x, i1 false)
  ret <8 x i16> %1
}

define <8 x i32> @__ocl_helper_ctz_v8u32(<8 x i32> %x) {
  %1 = tail call <8 x i32> @llvm.cttz.v8i32(<8 x i32> %x, i1 false)
  ret <8 x i32> %1
}

define <8 x i64> @__ocl_helper_ctz_v8u64(<8 x i64> %x) {
  %1 = tail call <8 x i64> @llvm.cttz.v8i64(<8 x i64> %x, i1 false)
  ret <8 x i64> %1
}

declare <16 x i8> @llvm.cttz.v16i8(<16 x i8>, i1) nounwind readonly
declare <16 x i16> @llvm.cttz.v16i16(<16 x i16>, i1) nounwind readonly
declare <16 x i32> @llvm.cttz.v16i32(<16 x i32>, i1) nounwind readonly
declare <16 x i64> @llvm.cttz.v16i64(<16 x i64>, i1) nounwind readonly

define <16 x i8> @__ocl_helper_ctz_v16u8(<16 x i8> %x) {
  %1 = tail call <16 x i8> @llvm.cttz.v16i8(<16 x i8> %x, i1 false)
  ret <16 x i8> %1
}

define <16 x i16> @__ocl_helper_ctz_v16u16(<16 x i16> %x) {
  %1 = tail call <16 x i16> @llvm.cttz.v16i16(<16 x i16> %x, i1 false)
  ret <16 x i16> %1
}

define <16 x i32> @__ocl_helper_ctz_v16u32(<16 x i32> %x) {
  %1 = tail call <16 x i32> @llvm.cttz.v16i32(<16 x i32> %x, i1 false)
  ret <16 x i32> %1
}

define <16 x i64> @__ocl_helper_ctz_v16u64(<16 x i64> %x) {
  %1 = tail call <16 x i64> @llvm.cttz.v16i64(<16 x i64> %x, i1 false)
  ret <16 x i64> %1
}

define void @__ocl_expand_mask_4x16(i16 %mask, i16* %mask0, i16* %mask1,
                                               i16* %mask2, i16* %mask3) {

  %imask = bitcast i16 %mask to <16 x i1>
  %mask_16x32 = sext <16 x i1>%imask to <16 x i32>
  %mask_16x32_0 = shufflevector <16 x i32> %mask_16x32, <16 x i32>undef, <16 x i32> <i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 1, i32 2, i32 2, i32 2, i32 2, i32 3, i32 3, i32 3, i32 3>
  %imask_0 = icmp ne <16 x i32> %mask_16x32_0, zeroinitializer
  %mask_0_flat = bitcast <16 x i1> %imask_0 to i16
  store i16 %mask_0_flat, i16* %mask0, align 1

  %mask_16x32_1 = shufflevector <16 x i32> %mask_16x32, <16 x i32>undef, <16 x i32> <i32 4, i32 4, i32 4, i32 4, i32 5, i32 5, i32 5, i32 5, i32 6, i32 6, i32 6, i32 6, i32 7, i32 7, i32 7, i32 7>
  %imask_1 = icmp ne <16 x i32> %mask_16x32_1, zeroinitializer
  %mask_1_flat = bitcast <16 x i1> %imask_1 to i16
  store i16 %mask_1_flat, i16* %mask1, align 1

  %mask_16x32_2 = shufflevector <16 x i32> %mask_16x32, <16 x i32>undef, <16 x i32> <i32 8, i32 8, i32 8, i32 8, i32 9, i32 9, i32 9, i32 9, i32 10, i32 10, i32 10, i32 10, i32 11, i32 11, i32 11, i32 11>
  %imask_2 = icmp ne <16 x i32> %mask_16x32_2, zeroinitializer
  %mask_2_flat = bitcast <16 x i1> %imask_2 to i16
  store i16 %mask_2_flat, i16* %mask2, align 1

  %mask_16x32_3 = shufflevector <16 x i32> %mask_16x32, <16 x i32>undef, <16 x i32> <i32 12, i32 12, i32 12, i32 12, i32 13, i32 13, i32 13, i32 13, i32 14, i32 14, i32 14, i32 14, i32 15, i32 15, i32 15, i32 15>
  %imask_3 = icmp ne <16 x i32> %mask_16x32_3, zeroinitializer
  %mask_3_flat = bitcast <16 x i1> %imask_3 to i16
  store i16 %mask_3_flat, i16* %mask3, align 1

  ret void
}

define i8 addrspace(1)* @__ocl_wpipe2ptr(%opencl.pipe_wo_t addrspace(1)* %p) {
  %1 = bitcast %opencl.pipe_wo_t addrspace(1)* %p to i8 addrspace(1)*
  ret i8 addrspace(1)* %1
}

define i8 addrspace(1)* @__ocl_rpipe2ptr(%opencl.pipe_ro_t addrspace(1)* %p) {
  %1 = bitcast %opencl.pipe_ro_t addrspace(1)* %p to i8 addrspace(1)*
  ret i8 addrspace(1)* %1
}

define i8 addrspace(1)* @__to_global(i8 addrspace(4)* %p) {
  %1 = addrspacecast i8 addrspace(4)* %p to i8 addrspace(1)*
  ret i8 addrspace(1)* %1
}

define i8 addrspace(3)* @__to_local(i8 addrspace(4)* %p) {
  %1 = addrspacecast i8 addrspace(4)* %p to i8 addrspace(3)*
  ret i8 addrspace(3)* %1
}

define i8* @__to_private(i8 addrspace(4)* %p) {
  %1 = addrspacecast i8 addrspace(4)* %p to i8*
  ret i8* %1
}

define %opencl.pipe_ro_t addrspace(1)* @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)* %p) {
  %1 = addrspacecast %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)* %p to %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(1)*
  %2 = bitcast %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(1)* %1 to %opencl.pipe_ro_t addrspace(1)* addrspace(1)*
  %3 = load %opencl.pipe_ro_t addrspace(1)*, %opencl.pipe_ro_t addrspace(1)* addrspace(1)* %2
  ret %opencl.pipe_ro_t addrspace(1)* %3
}

define %opencl.pipe_wo_t addrspace(1)* @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)* %p) {
  %1 = addrspacecast %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)* %p to %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(1)*
  %2 = bitcast %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(1)* %1 to %opencl.pipe_wo_t addrspace(1)* addrspace(1)*
  %3 = load %opencl.pipe_wo_t addrspace(1)*, %opencl.pipe_wo_t addrspace(1)* addrspace(1)* %2
  ret %opencl.pipe_wo_t addrspace(1)* %3
}

; Ballot (NEO-like)

define <16 x i32> @intel_sub_group_ballot_vf16(<16 x i1> %pred, <16 x i32> %vec_mask) #1 {
  %1 = icmp ne <16 x i32> %vec_mask, zeroinitializer
  %2 = and <16 x i1> %pred, %1
  %3 = bitcast <16 x i1> %2 to i16
  %conv = zext i16 %3 to i32
  %splat.splatinsert = insertelement <16 x i32> undef, i32 %conv, i32 0
  %splat.splat = shufflevector <16 x i32> %splat.splatinsert, <16 x i32> undef, <16 x i32> zeroinitializer
  ret <16 x i32> %splat.splat
}

define <8 x i32> @intel_sub_group_ballot_vf8(<8 x i1> %pred, <8 x i32> %vec_mask) #1 {
  %1 = icmp ne <8 x i32> %vec_mask, zeroinitializer
  %2 = and <8 x i1> %pred, %1
  %3 = bitcast <8 x i1> %2 to i8
  %conv = zext i8 %3 to i32
  %splat.splatinsert = insertelement <8 x i32> undef, i32 %conv, i32 0
  %splat.splat = shufflevector <8 x i32> %splat.splatinsert, <8 x i32> undef, <8 x i32> zeroinitializer
  ret <8 x i32> %splat.splat
}

define <4 x i32> @intel_sub_group_ballot_vf4(<4 x i1> %pred, <4 x i32> %vec_mask) #1 {
  %1 = icmp ne <4 x i32> %vec_mask, zeroinitializer
  %m = shufflevector <4 x i1> %1, <4 x i1> zeroinitializer, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %d = shufflevector <4 x i1> %pred, <4 x i1> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %2 = and <8 x i1> %m, %d
  %3 = bitcast <8 x i1> %2 to i8
  %conv = zext i8 %3 to i32
  %splat.splatinsert = insertelement <4 x i32> undef, i32 %conv, i32 0
  %splat.splat = shufflevector <4 x i32> %splat.splatinsert, <4 x i32> undef, <4 x i32> zeroinitializer
  ret <4 x i32> %splat.splat
}

attributes #1 = { norecurse nounwind readnone }
