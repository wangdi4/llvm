;/*=================================================================================
;Copyright (c) 2012, Intel Corporation
;Subject to the terms and conditions of the Master Development License
;Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
;OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
;==================================================================================*/

define signext i16 @__ocl_zext_v1i8_v1i16(i8 signext %x) nounwind {
  %1 = zext i8 %x to i16
  ret i16 %1
}

define signext i16 @__ocl_sext_v1i8_v1i16(i8 signext %x) nounwind {
  %1 = sext i8 %x to i16
  ret i16 %1
}

define signext i8 @__ocl_trunc_v1i16_v1i8(i16 signext %x) nounwind {
  %1 = trunc i16 %x to i8
  ret i8 %1
}

define i32 @__ocl_zext_v1i8_v1i32(i8 signext %x) nounwind {
  %1 = zext i8 %x to i32
  ret i32 %1
}

define i32 @__ocl_sext_v1i8_v1i32(i8 signext %x) nounwind {
  %1 = sext i8 %x to i32
  ret i32 %1
}

define signext i8 @__ocl_trunc_v1i32_v1i8(i32 %x) nounwind {
  %1 = trunc i32 %x to i8
  ret i8 %1
}

define i64 @__ocl_zext_v1i8_v1i64(i8 signext %x) nounwind {
  %1 = zext i8 %x to i64
  ret i64 %1
}

define i64 @__ocl_sext_v1i8_v1i64(i8 signext %x) nounwind {
  %1 = sext i8 %x to i64
  ret i64 %1
}

define signext i8 @__ocl_trunc_v1i64_v1i8(i64 %x) nounwind {
  %1 = trunc i64 %x to i8
  ret i8 %1
}

define i32 @__ocl_zext_v1i16_v1i32(i16 signext %x) nounwind {
  %1 = zext i16 %x to i32
  ret i32 %1
}

define i32 @__ocl_sext_v1i16_v1i32(i16 signext %x) nounwind {
  %1 = sext i16 %x to i32
  ret i32 %1
}

define signext i16 @__ocl_trunc_v1i32_v1i16(i32 %x) nounwind {
  %1 = trunc i32 %x to i16
  ret i16 %1
}

define i64 @__ocl_zext_v1i16_v1i64(i16 signext %x) nounwind {
  %1 = zext i16 %x to i64
  ret i64 %1
}

define i64 @__ocl_sext_v1i16_v1i64(i16 signext %x) nounwind {
  %1 = sext i16 %x to i64
  ret i64 %1
}

define signext i16 @__ocl_trunc_v1i64_v1i16(i64 %x) nounwind {
  %1 = trunc i64 %x to i16
  ret i16 %1
}

define i64 @__ocl_zext_v1i32_v1i64(i32 %x) nounwind {
  %1 = zext i32 %x to i64
  ret i64 %1
}

define i64 @__ocl_sext_v1i32_v1i64(i32 %x) nounwind {
  %1 = sext i32 %x to i64
  ret i64 %1
}

define i32 @__ocl_trunc_v1i64_v1i32(i64 %x) nounwind {
  %1 = trunc i64 %x to i32
  ret i32 %1
}

define <8 x i16> @__ocl_zext_v2i8_v2i16(<2 x i8>* byval align 8 %x) nounwind {
  %tmp = load <2 x i8>* %x, align 8
  %1 = zext <2 x i8> %tmp to <2 x i16>
  %ret = shufflevector <2 x i16> %1, <2 x i16> <i16 0, i16 undef>, <8 x i32> <i32 0, i32 1, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  ret <8 x i16> %ret
}

define <8 x i16> @__ocl_sext_v2i8_v2i16(<2 x i8>* byval align 8 %x) nounwind {
  %tmp = load <2 x i8>* %x, align 8
  %1 = sext <2 x i8> %tmp to <2 x i16>
  %ret = shufflevector <2 x i16> %1, <2 x i16> <i16 0, i16 undef>, <8 x i32> <i32 0, i32 1, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  ret <8 x i16> %ret
}

define <16 x i8> @__ocl_trunc_v2i16_v2i8(i32 %x) nounwind {
  %tmp = bitcast i32 %x to < 2 x i16>
  %1 = trunc <2 x i16> %tmp to <2 x i8>
  %ret = shufflevector <2 x i8> %1, <2 x i8> <i8 0, i8 undef>, <16 x i32> <i32 0, i32 1, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  ret <16 x i8> %ret
}

define <4 x i32> @__ocl_zext_v2i8_v2i32(<2 x i8>* byval align 8 %x) nounwind {
  %tmp = load <2 x i8>* %x, align 8
  %1 = zext <2 x i8> %tmp to <2 x i32>
  %ret = shufflevector <2 x i32> %1, <2 x i32> <i32 0, i32 undef>, <4 x i32> <i32 0, i32 1, i32 2, i32 2>
  ret <4 x i32> %ret
}

define <4 x i32> @__ocl_sext_v2i8_v2i32(<2 x i8>* byval align 8 %x) nounwind {
  %tmp = load <2 x i8>* %x, align 8
  %1 = sext <2 x i8> %tmp to <2 x i32>
  %ret = shufflevector <2 x i32> %1, <2 x i32> <i32 0, i32 undef>, <4 x i32> <i32 0, i32 1, i32 2, i32 2>
  ret <4 x i32> %ret
}

define <16 x i8> @__ocl_trunc_v2i32_v2i8(double %x) nounwind {
  %tmp = bitcast double %x to <2 x i32>
  %1 = trunc <2 x i32> %tmp to <2 x i8>
  %ret = shufflevector <2 x i8> %1, <2 x i8> <i8 0, i8 undef>, <16 x i32> <i32 0, i32 1, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  ret <16 x i8> %ret
}

define <2 x i64> @__ocl_zext_v2i8_v2i64(<2 x i8>* byval align 8 %x) nounwind {
  %tmp = load <2 x i8>* %x, align 8
  %1 = zext <2 x i8> %tmp to <2 x i64>
  ret <2 x i64> %1
}

define <2 x i64> @__ocl_sext_v2i8_v2i64(<2 x i8>* byval align 8 %x) nounwind {
  %tmp = load <2 x i8>* %x, align 8
  %1 = sext <2 x i8> %tmp to <2 x i64>
  ret <2 x i64> %1
}

define <16 x i8> @__ocl_trunc_v2i64_v2i8(<2 x i64> %x) nounwind {
  %1 = trunc <2 x i64> %x to <2 x i8>
  %ret = shufflevector <2 x i8> %1, <2 x i8> <i8 0, i8 undef>, <16 x i32> <i32 0, i32 1, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  ret <16 x i8> %ret
}

define <4 x i32> @__ocl_zext_v2i16_v2i32(i32 %x) nounwind {
  %tmp = bitcast i32 %x to <2 x i16>
  %1 = zext <2 x i16> %tmp to <2 x i32>
  %ret = shufflevector <2 x i32> %1, <2 x i32> <i32 0, i32 undef>, <4 x i32> <i32 0, i32 1, i32 2, i32 2>
  ret <4 x i32> %ret
}

define <4 x i32> @__ocl_sext_v2i16_v2i32(i32 %x) nounwind {
  %tmp = bitcast i32 %x to <2 x i16>
  %1 = sext <2 x i16> %tmp to <2 x i32>
  %ret = shufflevector <2 x i32> %1, <2 x i32> <i32 0, i32 undef>, <4 x i32> <i32 0, i32 1, i32 2, i32 2>
  ret <4 x i32> %ret
}

define <8 x i16> @__ocl_trunc_v2i32_v2i16(double %x) nounwind {
  %tmp = bitcast double %x to <2 x i32>
  %1 = trunc <2 x i32> %tmp to <2 x i16>
  %ret = shufflevector <2 x i16> %1, <2 x i16> <i16 0, i16 undef>, <8 x i32> <i32 0, i32 1, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  ret <8 x i16> %ret
}

define <2 x i64> @__ocl_zext_v2i16_v2i64(i32 %x) nounwind {
  %tmp = bitcast i32 %x to <2 x i16>
  %1 = zext <2 x i16> %tmp to <2 x i64>
  ret <2 x i64> %1
}

define <2 x i64> @__ocl_sext_v2i16_v2i64(i32 %x) nounwind {
  %tmp = bitcast i32 %x to <2 x i16>
  %1 = sext <2 x i16> %tmp to <2 x i64>
  ret <2 x i64> %1
}

define <8 x i16> @__ocl_trunc_v2i64_v2i16(<2 x i64> %x) nounwind {
  %1 = trunc <2 x i64> %x to <2 x i16>
  %ret = shufflevector <2 x i16> %1, <2 x i16> <i16 0, i16 undef>, <8 x i32> <i32 0, i32 1, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  ret <8 x i16> %ret
}

define <2 x i64> @__ocl_zext_v2i32_v2i64(double %x) nounwind {
  %tmp = bitcast double %x to <2 x i32>
  %1 = zext <2 x i32> %tmp to <2 x i64>
  ret <2 x i64> %1
}

define <2 x i64> @__ocl_sext_v2i32_v2i64(double %x) nounwind {
  %tmp = bitcast double %x to <2 x i32>
  %1 = sext <2 x i32> %tmp to <2 x i64>
  ret <2 x i64> %1
}

define <4 x i32> @__ocl_trunc_v2i64_v2i32(<2 x i64> %x) nounwind {
  %1 = trunc <2 x i64> %x to <2 x i32>
  %ret = shufflevector <2 x i32> %1, <2 x i32> <i32 0, i32 undef>, <4 x i32> <i32 0, i32 1, i32 2, i32 2>
  ret <4 x i32> %ret
}

define <8 x i16> @__ocl_zext_v3i8_v3i16(i32 %x) nounwind {
  %tmp = bitcast i32 %x to <4 x i8>
  %1 = zext <4 x i8> %tmp to <4 x i16>
  %ret = shufflevector <4 x i16> %1, <4 x i16> <i16 0, i16 undef, i16 undef, i16 undef>, <8 x i32> <i32 0, i32 1, i32 2, i32 4, i32 4, i32 4, i32 4, i32 4>
  ret <8 x i16> %ret
}

define <8 x i16> @__ocl_sext_v3i8_v3i16(i32 %x) nounwind {
  %tmp = bitcast i32 %x to <4 x i8>
  %1 = sext <4 x i8> %tmp to <4 x i16>
  %ret = shufflevector <4 x i16> %1, <4 x i16> <i16 0, i16 undef, i16 undef, i16 undef>, <8 x i32> <i32 0, i32 1, i32 2, i32 4, i32 4, i32 4, i32 4, i32 4>
  ret <8 x i16> %ret
}

define <16 x i8> @__ocl_trunc_v3i16_v3i8(double %x) nounwind {
  %tmp = bitcast double %x to <4 x i16>
  %1 = trunc <4 x i16> %tmp to <4 x i8>
  %ret = shufflevector <4 x i8> %1, <4 x i8> <i8 0, i8 undef, i8 undef, i8 undef>, <16 x i32> <i32 0, i32 1, i32 2, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>
  ret <16 x i8> %ret
}

define <4 x i32> @__ocl_zext_v3i8_v3i32(i32 %x) nounwind {
  %tmp = bitcast i32 %x to <4 x i8>
  %1 = zext <4 x i8> %tmp to <4 x i32>
  ret <4 x i32> %1
}

define <4 x i32> @__ocl_sext_v3i8_v3i32(i32 %x) nounwind {
  %tmp = bitcast i32 %x to <4 x i8>
  %1 = sext <4 x i8> %tmp to <4 x i32>
  ret <4 x i32> %1
}

define <16 x i8> @__ocl_trunc_v3i32_v3i8(<2 x double> %x) nounwind {
  %tmp = bitcast <2 x double> %x to <4 x i32>
  %1 = trunc <4 x i32> %tmp to <4 x i8>
  %ret = shufflevector <4 x i8> %1, <4 x i8> <i8 0, i8 undef, i8 undef, i8 undef>, <16 x i32> <i32 0, i32 1, i32 2, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>
  ret <16 x i8> %ret
}

define <3 x i64> @__ocl_zext_v3i8_v3i64(i32 %x) nounwind {
  %tmp = bitcast i32 %x to <4 x i8>
  %1 = zext <4 x i8> %tmp to <4 x i64>
  %ret = shufflevector <4  x i64> %1, <4 x i64> undef, <3 x i32> <i32 0, i32 1, i32 2>
  ret <3 x i64> %ret
}

define <3 x i64> @__ocl_sext_v3i8_v3i64(i32 %x) nounwind {
  %tmp = bitcast i32 %x to <4 x i8>
  %1 = sext <4 x i8> %tmp to <4 x i64>
  %ret = shufflevector <4  x i64> %1, <4 x i64> undef, <3 x i32> <i32 0, i32 1, i32 2>
  ret <3 x i64> %ret
}

define <16 x i8> @__ocl_trunc_v3i64_v3i8(<3 x i64> %x) nounwind {
  %tmp = shufflevector <3  x i64> %x, <3 x i64> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %1 = trunc <4 x i64> %tmp to <4 x i8>
  %ret = shufflevector <4 x i8> %1, <4 x i8> <i8 0, i8 undef, i8 undef, i8 undef>, <16 x i32> <i32 0, i32 1, i32 2, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>
  ret <16 x i8> %ret
}

define <4 x i32> @__ocl_zext_v3i16_v3i32(double %x) nounwind {
  %tmp = bitcast double %x to <4 x i16>
  %1 = zext <4 x i16> %tmp to <4 x i32>
  ret <4 x i32> %1
}

define <4 x i32> @__ocl_sext_v3i16_v3i32(double %x) nounwind {
  %tmp = bitcast double %x to <4 x i16>
  %1 = sext <4 x i16> %tmp to <4 x i32>
  ret <4 x i32> %1
}

define <8 x i16> @__ocl_trunc_v3i32_v3i16(<2 x double> %x) nounwind {
  %tmp = bitcast <2 x double> %x to <4 x i32>
  %1 = trunc <4 x i32> %tmp to <4 x i16>
  %ret = shufflevector <4 x i16> %1, <4 x i16> <i16 0, i16 undef, i16 undef, i16 undef>, <8 x i32> <i32 0, i32 1, i32 2, i32 4, i32 4, i32 4, i32 4, i32 4>
  ret <8 x i16> %ret
}

define <3 x i64> @__ocl_zext_v3i16_v3i64(double %x) nounwind {
  %tmp = bitcast double %x to <4 x i16>
  %1 = zext <4 x i16> %tmp to <4 x i64>
  %ret = shufflevector <4  x i64> %1, <4 x i64> undef, <3 x i32> <i32 0, i32 1, i32 2>
  ret <3 x i64> %ret
}

define <3 x i64> @__ocl_sext_v3i16_v3i64(double %x) nounwind {
  %tmp = bitcast double %x to <4 x i16>
  %1 = sext <4 x i16> %tmp to <4 x i64>
  %ret = shufflevector <4  x i64> %1, <4 x i64> undef, <3 x i32> <i32 0, i32 1, i32 2>
  ret <3 x i64> %ret
}

define <8 x i16> @__ocl_trunc_v3i64_v3i16(<3 x i64> %x) nounwind {
  %tmp = shufflevector <3  x i64> %x, <3 x i64> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %1 = trunc <4 x i64> %tmp to <4 x i16>
  %ret = shufflevector <4 x i16> %1, <4 x i16> <i16 0, i16 undef, i16 undef, i16 undef>, <8 x i32> <i32 0, i32 1, i32 2, i32 4, i32 4, i32 4, i32 4, i32 4>
  ret <8 x i16> %ret
}

define <3 x i64> @__ocl_zext_v3i32_v3i64(<2 x double> %x) nounwind {
  %tmp = bitcast < 2 x double> %x to <4 x i32>
  %1 = zext <4 x i32> %tmp to <4 x i64>
  %ret = shufflevector <4  x i64> %1, <4 x i64> undef, <3 x i32> <i32 0, i32 1, i32 2>
  ret <3 x i64> %ret
}

define <3 x i64> @__ocl_sext_v3i32_v3i64(<2 x double> %x) nounwind {
  %tmp = bitcast < 2 x double> %x to <4 x i32>
  %1 = sext <4 x i32> %tmp to <4 x i64>
  %ret = shufflevector <4  x i64> %1, <4 x i64> undef, <3 x i32> <i32 0, i32 1, i32 2>
  ret <3 x i64> %ret
}

define <4 x i32> @__ocl_trunc_v3i64_v3i32(<3 x i64> %x) nounwind {
  %tmp = shufflevector <3  x i64> %x, <3 x i64> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %1 = trunc <4 x i64> %tmp to <4 x i32>
  ret <4 x i32> %1
}

define <8 x i16> @__ocl_zext_v4i8_v4i16(i32 %x) nounwind {
  %tmp = bitcast i32 %x to <4 x i8>
  %1 = zext <4 x i8> %tmp to <4 x i16>
  %ret = shufflevector <4 x i16> %1, <4 x i16> <i16 0, i16 undef, i16 undef, i16 undef>, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 4, i32 4, i32 4>
  ret <8 x i16> %ret
}

define <8 x i16> @__ocl_sext_v4i8_v4i16(i32 %x) nounwind {
  %tmp = bitcast i32 %x to <4 x i8>
  %1 = sext <4 x i8> %tmp to <4 x i16>
  %ret = shufflevector <4 x i16> %1, <4 x i16> <i16 0, i16 undef, i16 undef, i16 undef>, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 4, i32 4, i32 4>
  ret <8 x i16> %ret
}

define <16 x i8> @__ocl_trunc_v4i16_v4i8(double %x) nounwind {
  %tmp = bitcast double %x to <4 x i16>
  %1 = trunc <4 x i16> %tmp to <4 x i8>
  %ret = shufflevector <4 x i8> %1, <4 x i8> <i8 0, i8 undef, i8 undef, i8 undef>, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>
  ret <16 x i8> %ret
}

define <4 x i32> @__ocl_zext_v4i8_v4i32(i32 %x) nounwind {
  %tmp = bitcast i32 %x to <4 x i8>
  %1 = zext <4 x i8> %tmp to <4 x i32>
  ret <4 x i32> %1
}

define <4 x i32> @__ocl_sext_v4i8_v4i32(i32 %x) nounwind {
  %tmp = bitcast i32 %x to <4 x i8>
  %1 = sext <4 x i8> %tmp to <4 x i32>
  ret <4 x i32> %1
}

define <16 x i8> @__ocl_trunc_v4i32_v4i8(<4 x i32> %x) nounwind {
  %1 = trunc <4 x i32> %x to <4 x i8>
  %ret = shufflevector <4 x i8> %1, <4 x i8> <i8 0, i8 undef, i8 undef, i8 undef>, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>
  ret <16 x i8> %ret
}

define <4 x i64> @__ocl_zext_v4i8_v4i64(i32 %x) nounwind {
  %tmp = bitcast i32 %x to <4 x i8>
  %1 = zext <4 x i8> %tmp to <4 x i64>
  ret <4 x i64> %1
}

define <4 x i64> @__ocl_sext_v4i8_v4i64(i32 %x) nounwind {
  %tmp = bitcast i32 %x to <4 x i8>
  %1 = sext <4 x i8> %tmp to <4 x i64>
  ret <4 x i64> %1
}

define <16 x i8> @__ocl_trunc_v4i64_v4i8(<4 x i64> %x) nounwind {
  %1 = trunc <4 x i64> %x to <4 x i8>
  %ret = shufflevector <4 x i8> %1, <4 x i8> <i8 0, i8 undef, i8 undef, i8 undef>, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>
  ret <16 x i8> %ret
}

define <4 x i32> @__ocl_zext_v4i16_v4i32(double %x) nounwind {
  %tmp = bitcast double %x to <4 x i16>
  %1 = zext <4 x i16> %tmp to <4 x i32>
  ret <4 x i32> %1
}

define <4 x i32> @__ocl_sext_v4i16_v4i32(double %x) nounwind {
  %tmp = bitcast double %x to <4 x i16>
  %1 = sext <4 x i16> %tmp to <4 x i32>
  ret <4 x i32> %1
}

define <8 x i16> @__ocl_trunc_v4i32_v4i16(<4 x i32> %x) nounwind {
  %1 = trunc <4 x i32> %x to <4 x i16>
  %ret = shufflevector <4 x i16> %1, <4 x i16> <i16 0, i16 undef, i16 undef, i16 undef>, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 4, i32 4, i32 4>
  ret <8 x i16> %ret
}

define <4 x i64> @__ocl_zext_v4i16_v4i64(double %x) nounwind {
  %tmp = bitcast double %x to <4 x i16>
  %1 = zext <4 x i16> %tmp to <4 x i64>
  ret <4 x i64> %1
}

define <4 x i64> @__ocl_sext_v4i16_v4i64(double %x) nounwind {
  %tmp = bitcast double %x to <4 x i16>
  %1 = sext <4 x i16> %tmp to <4 x i64>
  ret <4 x i64> %1
}

define <8 x i16> @__ocl_trunc_v4i64_v4i16(<4 x i64> %x) nounwind {
  %1 = trunc <4 x i64> %x to <4 x i16>
  %ret = shufflevector <4 x i16> %1, <4 x i16> <i16 0, i16 undef, i16 undef, i16 undef>, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 4, i32 4, i32 4>
  ret <8 x i16> %ret
}

define <4 x i64> @__ocl_zext_v4i32_v4i64(<4 x i32> %x) nounwind {
  %1 = zext <4 x i32> %x to <4 x i64>
  ret <4 x i64> %1
}

define <4 x i64> @__ocl_sext_v4i32_v4i64(<4 x i32> %x) nounwind {
  %1 = sext <4 x i32> %x to <4 x i64>
  ret <4 x i64> %1
}

define <4 x i32> @__ocl_trunc_v4i64_v4i32(<4 x i64> %x) nounwind {
  %1 = trunc <4 x i64> %x to <4 x i32>
  ret <4 x i32> %1
}

define <8 x i16> @__ocl_zext_v8i8_v8i16(double %x) nounwind {
  %tmp = bitcast double %x to <8 x i8>
  %1 = zext <8 x i8> %tmp to <8 x i16>
  ret <8 x i16> %1
}

define <8 x i16> @__ocl_sext_v8i8_v8i16(double %x) nounwind {
  %tmp = bitcast double %x to <8 x i8>
  %1 = sext <8 x i8> %tmp to <8 x i16>
  ret <8 x i16> %1
}

define <16 x i8> @__ocl_trunc_v8i16_v8i8(<8 x i16> %x) nounwind {
  %1 = trunc <8 x i16> %x to <8 x i8>
  %ret = shufflevector <8 x i8> %1, <8 x i8> <i8 0, i8 undef, i8 undef, i8 undef, i8 undef, i8 undef, i8 undef, i8 undef>, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8>
  ret <16 x i8> %ret
}

define <8 x i32> @__ocl_zext_v8i8_v8i32(double %x) nounwind {
  %tmp = bitcast double %x to <8 x i8>
  %1 = zext <8 x i8> %tmp to <8 x i32>
  ret <8 x i32> %1
}

define <8 x i32> @__ocl_sext_v8i8_v8i32(double %x) nounwind {
  %tmp = bitcast double %x to <8 x i8>
  %1 = sext <8 x i8> %tmp to <8 x i32>
  ret <8 x i32> %1
}

define <16 x i8> @__ocl_trunc_v8i32_v8i8(<8 x i32> %x) nounwind {
  %1 = trunc <8 x i32> %x to <8 x i8>
  %ret = shufflevector <8 x i8> %1, <8 x i8> <i8 0, i8 undef, i8 undef, i8 undef, i8 undef, i8 undef, i8 undef, i8 undef>, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8>
  ret <16 x i8> %ret
}

define <8 x i64> @__ocl_zext_v8i8_v8i64(double %x) nounwind {
  %tmp = bitcast double %x to <8 x i8>
  %1 = zext <8 x i8> %tmp to <8 x i64>
  ret <8 x i64> %1
}

define <8 x i64> @__ocl_sext_v8i8_v8i64(double %x) nounwind {
  %tmp = bitcast double %x to <8 x i8>
  %1 = sext <8 x i8> %tmp to <8 x i64>
  ret <8 x i64> %1
}

define <16 x i8> @__ocl_trunc_v8i64_v8i8(<8 x i64>* byval align 64 %x) nounwind {
  %tmp = load <8 x i64>* %x, align 64
  %1 = trunc <8 x i64> %tmp to <8 x i8>
  %ret = shufflevector <8 x i8> %1, <8 x i8> <i8 0, i8 undef, i8 undef, i8 undef, i8 undef, i8 undef, i8 undef, i8 undef>, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8>
  ret <16 x i8> %ret
}

define <8 x i32> @__ocl_zext_v8i16_v8i32(<8 x i16> %x) nounwind {
  %1 = zext <8 x i16> %x to <8 x i32>
  ret <8 x i32> %1
}

define <8 x i32> @__ocl_sext_v8i16_v8i32(<8 x i16> %x) nounwind {
  %1 = sext <8 x i16> %x to <8 x i32>
  ret <8 x i32> %1
}

define <8 x i16> @__ocl_trunc_v8i32_v8i16(<8 x i32> %x) nounwind {
  %1 = trunc <8 x i32> %x to <8 x i16>
  ret <8 x i16> %1
}

define <8 x i64> @__ocl_zext_v8i16_v8i64(<8 x i16> %x) nounwind {
  %1 = zext <8 x i16> %x to <8 x i64>
  ret <8 x i64> %1
}

define <8 x i64> @__ocl_sext_v8i16_v8i64(<8 x i16> %x) nounwind {
  %1 = sext <8 x i16> %x to <8 x i64>
  ret <8 x i64> %1
}

define <8 x i16> @__ocl_trunc_v8i64_v8i16(<8 x i64>* byval align 64 %x) nounwind {
  %tmp = load <8 x i64>* %x, align 64
  %1 = trunc <8 x i64> %tmp to <8 x i16>
  ret <8 x i16> %1
}

define <8 x i64> @__ocl_zext_v8i32_v8i64(<8 x i32> %x) nounwind {
  %1 = zext <8 x i32> %x to <8 x i64>
  ret <8 x i64> %1
}

define <8 x i64> @__ocl_sext_v8i32_v8i64(<8 x i32> %x) nounwind {
  %1 = sext <8 x i32> %x to <8 x i64>
  ret <8 x i64> %1
}

define <8 x i32> @__ocl_trunc_v8i64_v8i32(<8 x i64>* byval align 64 %x) nounwind {
  %tmp = load <8 x i64>* %x, align 64
  %1 = trunc <8 x i64> %tmp to <8 x i32>
  ret <8 x i32> %1
}

define <16 x i16> @__ocl_zext_v16i8_v16i16(<16 x i8> %x) nounwind {
  %1 = zext <16 x i8> %x to <16 x i16>
  ret <16 x i16> %1
}

define <16 x i16> @__ocl_sext_v16i8_v16i16(<16 x i8> %x) nounwind {
  %1 = sext <16 x i8> %x to <16 x i16>
  ret <16 x i16> %1
}

define <16 x i8> @__ocl_trunc_v16i16_v16i8(<16 x i16> %x) nounwind {
  %1 = trunc <16 x i16> %x to <16 x i8>
  ret <16 x i8> %1
}

define <16 x i32> @__ocl_zext_v16i8_v16i32(<16 x i8> %x) nounwind {
  %1 = zext <16 x i8> %x to <16 x i32>
  ret <16 x i32> %1
}

define <16 x i32> @__ocl_sext_v16i8_v16i32(<16 x i8> %x) nounwind {
  %1 = sext <16 x i8> %x to <16 x i32>
  ret <16 x i32> %1
}

define <16 x i8> @__ocl_trunc_v16i32_v16i8(<16 x i32>* byval align 64 %x) nounwind {
  %tmp = load <16 x i32>* %x, align 64
  %1 = trunc <16 x i32> %tmp to <16 x i8>
  ret <16 x i8> %1
}

define void @__ocl_zext_v16i8_v16i64(<16 x i64>* %sret, <16 x i8> %x) nounwind {
  %1 = zext <16 x i8> %x to <16 x i64>
  store <16 x i64> %1, <16 x i64>* %sret, align 128
  ret void
}

define void @__ocl_sext_v16i8_v16i64(<16 x i64>* %sret, <16 x i8> %x) nounwind {
  %1 = sext <16 x i8> %x to <16 x i64>
  store <16 x i64> %1, <16 x i64>* %sret, align 128
  ret void
}

define <16 x i8> @__ocl_trunc_v16i64_v16i8(<16 x i64>* byval align 128 %x) nounwind {
  %tmp = load <16 x i64>* %x, align 128
  %1 = trunc <16 x i64> %tmp to <16 x i8>
  ret <16 x i8> %1
}

define <16 x i32> @__ocl_zext_v16i16_v16i32(<16 x i16> %x) nounwind {
  %1 = zext <16 x i16> %x to <16 x i32>
  ret <16 x i32> %1
}

define <16 x i32> @__ocl_sext_v16i16_v16i32(<16 x i16> %x) nounwind {
  %1 = sext <16 x i16> %x to <16 x i32>
  ret <16 x i32> %1
}

define <16 x i16> @__ocl_trunc_v16i32_v16i16(<16 x i32>* byval align 64 %x) nounwind {
  %tmp = load <16 x i32>* %x, align 64
  %1 = trunc <16 x i32> %tmp to <16 x i16>
  ret <16 x i16> %1
}

define void @__ocl_zext_v16i16_v16i64(<16 x i64>* %sret, <16 x i16> %x) nounwind {
  %1 = zext <16 x i16> %x to <16 x i64>
  store <16 x i64> %1, <16 x i64>* %sret, align 128
  ret void
}

define void @__ocl_sext_v16i16_v16i64(<16 x i64>* %sret, <16 x i16> %x) nounwind {
  %1 = sext <16 x i16> %x to <16 x i64>
  store <16 x i64> %1, <16 x i64>* %sret, align 128
  ret void
}

define <16 x i16> @__ocl_trunc_v16i64_v16i16(<16 x i64>* byval align 128 %x) nounwind {
  %tmp = load <16 x i64>* %x, align 128
  %1 = trunc <16 x i64> %tmp to <16 x i16>
  ret <16 x i16> %1
}

define void @__ocl_zext_v16i32_v16i64(<16 x i64>* %sret, <16 x i32>* byval align 64 %x) nounwind {
  %tmp = load <16 x i32>* %x, align 64
  %1 = zext <16 x i32> %tmp to <16 x i64>
  store <16 x i64> %1, <16 x i64>* %sret, align 128
  ret void
}

define void @__ocl_sext_v16i32_v16i64(<16 x i64>* %sret, <16 x i32>* byval align 64 %x) nounwind {
  %tmp = load <16 x i32>* %x, align 64
  %1 = sext <16 x i32> %tmp to <16 x i64>
  store <16 x i64> %1, <16 x i64>* %sret, align 128
  ret void
}

define <16 x i32> @__ocl_trunc_v16i64_v16i32(<16 x i64>* byval align 128 %x) nounwind {
  %tmp = load <16 x i64>* %x, align 128
  %1 = trunc <16 x i64> %tmp to <16 x i32>
  ret <16 x i32> %1
}

declare i8 @llvm.ctlz.i8(i8, i1) nounwind readonly
declare i16 @llvm.ctlz.i16(i16, i1) nounwind readonly
declare i32 @llvm.ctlz.i32(i32, i1) nounwind readonly
declare i64 @llvm.ctlz.i64(i64, i1) nounwind readonly

define i8 @__ocl_helper_clz_v1u8(i8 %x) nounwind {
  %1 = tail call i8 @llvm.ctlz.i8(i8 %x, i1 false)
  ret i8 %1
}

define i16 @__ocl_helper_clz_v1u16(i16 %x) nounwind {
  %1 = tail call i16 @llvm.ctlz.i16(i16 %x, i1 false)
  ret i16 %1
}

define i32 @__ocl_helper_clz_v1u32(i32 %x) nounwind {
  %1 = tail call i32 @llvm.ctlz.i32(i32 %x, i1 false)
  ret i32 %1
}

define i64 @__ocl_helper_clz_v1u64(i64 %x) nounwind {
  %1 = tail call i64 @llvm.ctlz.i64(i64 %x, i1 false)
  ret i64 %1
}

declare i8 @llvm.cttz.i8(i8, i1) nounwind readonly
declare i16 @llvm.cttz.i16(i16, i1) nounwind readonly
declare i32 @llvm.cttz.i32(i32, i1) nounwind readonly
declare i64 @llvm.cttz.i64(i64, i1) nounwind readonly

define i8 @__ocl_helper_ctz_v1u8(i8 %x) nounwind {
  %1 = tail call i8 @llvm.cttz.i8(i8 %x, i1 false)
  ret i8 %1
}

define i16 @__ocl_helper_ctz_v1u16(i16 %x) nounwind {
  %1 = tail call i16 @llvm.cttz.i16(i16 %x, i1 false)
  ret i16 %1
}

define i32 @__ocl_helper_ctz_v1u32(i32 %x) nounwind {
  %1 = tail call i32 @llvm.cttz.i32(i32 %x, i1 false)
  ret i32 %1
}

define i64 @__ocl_helper_ctz_v1u64(i64 %x) nounwind {
  %1 = tail call i64 @llvm.cttz.i64(i64 %x, i1 false)
  ret i64 %1
}
