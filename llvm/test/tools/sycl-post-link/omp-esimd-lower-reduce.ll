; RUN: sycl-post-link -ompoffload-link-entries -ompoffload-explicit-simd --ir-output-only %s -S -o - | FileCheck %s

define dso_local spir_kernel i64 @test_v2i64(<2 x i64> %a0) !omp_simd_kernel !0 {
; CHECK: [[TMP1:%.*]] = extractelement <2 x i64> %a0, i32 0
; CHECK-NEXT: [[TMP2:%.*]] = extractelement <2 x i64> %a0, i32 1
; CHECK-NEXT: [[RES:%.*]] = add i64 [[TMP1]], [[TMP2]]
  %1 = call i64 @llvm.vector.reduce.add.v2i64(<2 x i64> %a0)
  ret i64 %1
}

define dso_local spir_kernel i32 @test_v8i32(<8 x i32> %a0) !omp_simd_kernel !0 {
; CHECK: [[TMP1:%.*]] = call <4 x i32> @llvm.genx.rdregioni.v4i32.v8i32.i16(<8 x i32> %a0, i32 0, i32 4, i32 1, i16 0, i32 0)
; CHECK-NEXT: [[TMP2:%.*]] = call <4 x i32> @llvm.genx.rdregioni.v4i32.v8i32.i16(<8 x i32> %a0, i32 0, i32 4, i32 1, i16 16, i32 0)
; CHECK-NEXT: [[TMP3:%.*]] = add <4 x i32> [[TMP1]], [[TMP2]]
; CHECK-NEXT: [[TMP4:%.*]] = call <2 x i32> @llvm.genx.rdregioni.v2i32.v4i32.i16(<4 x i32> [[TMP3]], i32 0, i32 2, i32 1, i16 0, i32 0)
; CHECK-NEXT: [[TMP5:%.*]] = call <2 x i32> @llvm.genx.rdregioni.v2i32.v4i32.i16(<4 x i32> [[TMP3]], i32 0, i32 2, i32 1, i16 8, i32 0)
; CHECK-NEXT: [[TMP6:%.*]] = add <2 x i32> [[TMP4]], [[TMP5]]
; CHECK-NEXT: [[TMP7:%.*]] = extractelement <2 x i32> [[TMP6]], i32 0
; CHECK-NEXT: [[TMP8:%.*]] = extractelement <2 x i32> [[TMP6]], i32 1
; CHECK-NEXT: [[RES:%.*]] = add i32 [[TMP7]], [[TMP8]]
  %1 = call i32 @llvm.vector.reduce.add.v8i32(<8 x i32> %a0)
  ret i32 %1
}

define dso_local spir_kernel i32 @test_xor_v8i32(<8 x i32> %a0) !omp_simd_kernel !0 {
; CHECK: [[TMP1:%.*]] = call <4 x i32> @llvm.genx.rdregioni.v4i32.v8i32.i16
; CHECK-NEXT: [[TMP2:%.*]] = call <4 x i32> @llvm.genx.rdregioni.v4i32.v8i32.i16
; CHECK-NEXT: [[TMP3:%.*]] = xor <4 x i32> [[TMP1]], [[TMP2]]
; CHECK-NEXT: [[TMP4:%.*]] = call <2 x i32> @llvm.genx.rdregioni.v2i32.v4i32.i16
; CHECK-NEXT: [[TMP5:%.*]] = call <2 x i32> @llvm.genx.rdregioni.v2i32.v4i32.i16
; CHECK-NEXT: [[TMP6:%.*]] = xor <2 x i32> [[TMP4]], [[TMP5]]
; CHECK-NEXT: [[TMP7:%.*]] = extractelement <2 x i32> [[TMP6]], i32 0
; CHECK-NEXT: [[TMP8:%.*]] = extractelement <2 x i32> [[TMP6]], i32 1
; CHECK-NEXT: [[RES:%.*]] = xor i32 [[TMP7]], [[TMP8]]
  %1 = call i32 @llvm.vector.reduce.xor.v8i32(<8 x i32> %a0)
  ret i32 %1
}

define dso_local spir_kernel i32 @test_non_pow2(<9 x i32> %a0) !omp_simd_kernel !0 {
; CHECK: [[TMP1:%.*]] = call <4 x i32> @llvm.genx.rdregioni.v4i32.v9i32.i16
; CHECK-NEXT: [[TMP2:%.*]] = call <4 x i32> @llvm.genx.rdregioni.v4i32.v9i32.i16
; CHECK-NEXT: [[TMP3:%.*]] = mul <4 x i32> [[TMP1]], [[TMP2]]
; CHECK-NEXT: [[TMP4:%.*]] = call <2 x i32> @llvm.genx.rdregioni.v2i32.v4i32.i16(<4 x i32> [[TMP3:%.*]]
; CHECK-NEXT: [[TMP5:%.*]] = call <2 x i32> @llvm.genx.rdregioni.v2i32.v4i32.i16(<4 x i32> [[TMP3:%.*]]
; CHECK-NEXT: [[TMP6:%.*]] = mul <2 x i32> [[TMP4:%.*]], [[TMP5:%.*]]
; CHECK-NEXT: [[TMP7:%.*]] = extractelement <2 x i32> [[TMP6:%.*]]
; CHECK-NEXT: [[TMP8:%.*]] = extractelement <2 x i32> [[TMP6:%.*]]
; CHECK-NEXT: [[TMP9:%.*]] = mul i32 [[TMP7]], [[TMP8]]
; CHECK-NEXT: [[REMAINDER:%.*]] = extractelement <9 x i32> %a0, i32 8
; CHECK-NEXT: [[RES:%.*]] = mul i32 [[TMP9]], [[REMAINDER]]
  %1 = call i32 @llvm.vector.reduce.mul.v9i32(<9 x i32> %a0)
  ret i32 %1
}

define dso_local spir_kernel i32 @test_non_pow2_2(<7 x i32> %a0) !omp_simd_kernel !0 {
; CHECK: [[TMP1:%.*]] = call <2 x i32> @llvm.genx.rdregioni.v2i32.v7i32.i16
; CHECK-NEXT: [[TMP2:%.*]] = call <2 x i32> @llvm.genx.rdregioni.v2i32.v7i32.i16
; CHECK-NEXT: [[TMP3:%.*]] = or <2 x i32> [[TMP1]], [[TMP2]]
; CHECK-NEXT: [[TMP4:%.*]] = call <2 x i32> @llvm.genx.rdregioni.v2i32.v2i32.i16(<2 x i32> [[TMP3]]
; CHECK-NEXT: [[REDUCE_ACROSS:%.*]] = call <2 x i32> @llvm.genx.rdregioni.v2i32.v7i32.i16(<7 x i32> %a0, i32 0, i32 2, i32 1, i16 16, i32 0)
; CHECK-NEXT: [[TMP5:%.*]] = or <2 x i32> [[TMP4]], [[REDUCE_ACROSS]]
  %1 = call i32 @llvm.vector.reduce.or.v7i32(<7 x i32> %a0)
  ret i32 %1
}

define dso_local spir_kernel float @test_float_reassoc(<8 x float> %a0) !omp_simd_kernel !0 {
; CHECK: [[TMP1:%.*]] = call <4 x float> @llvm.genx.rdregionf.v4f32.v8f32.i16
; CHECK-NEXT: [[TMP2:%.*]] = call <4 x float> @llvm.genx.rdregionf.v4f32.v8f32.i16
; CHECK-NEXT: [[TMP3:%.*]] = fadd <4 x float> [[TMP1]], [[TMP2]]
; CHECK-NEXT: [[TMP4:%.*]] = call <2 x float> @llvm.genx.rdregionf.v2f32.v4f32.i16(<4 x float> [[TMP3]]
; CHECK-NEXT: [[TMP5:%.*]] = call <2 x float> @llvm.genx.rdregionf.v2f32.v4f32.i16(<4 x float> [[TMP3]]
; CHECK-NEXT: [[TMP6:%.*]] = fadd <2 x float> [[TMP4]], [[TMP5]]
; CHECK-NEXT: [[TMP7:%.*]] = extractelement <2 x float> [[TMP6]], i32 0
; CHECK-NEXT: [[TMP8:%.*]] = extractelement <2 x float> [[TMP6]], i32 1
; CHECK-NEXT: [[TMP9:%.*]] = fadd float [[TMP7]], [[TMP8]]
; CHECK-NEXT: [[TMP10:%.*]] = fadd float [[TMP9]], -0.000000e+00
; CHECK-NEXT: ret float [[TMP10]]
  %res = call reassoc float @llvm.vector.reduce.fadd.v8f32(float -0.000000e+00, <8 x float> %a0)
  ret float %res
}

define dso_local spir_kernel float @test_float_no_reassoc(<8 x float> %a0) !omp_simd_kernel !0 {
; CHECK: [[TMP1:%.*]] = extractelement <8 x float> %a0, i32 0
; CHECK-NEXT: [[TMP2:%.*]] = fadd float -0.000000e+00, [[TMP1]]
; CHECK-NEXT: [[TMP3:%.*]] = extractelement <8 x float> %a0, i32 1
; CHECK-NEXT: [[TMP4:%.*]] = fadd float [[TMP2]], [[TMP3]]
; CHECK-NEXT: [[TMP5:%.*]] = extractelement <8 x float> %a0, i32 2
; CHECK-NEXT: [[TMP6:%.*]] = fadd float [[TMP4]], [[TMP5]]
; CHECK-NEXT: [[TMP7:%.*]] = extractelement <8 x float> %a0, i32 3
; CHECK-NEXT: [[TMP8:%.*]] = fadd float [[TMP6]], [[TMP7]]
; CHECK-NEXT: [[TMP9:%.*]] = extractelement <8 x float> %a0, i32 4
; CHECK-NEXT: [[TMP10:%.*]] = fadd float [[TMP8]], [[TMP9]]
; CHECK-NEXT: [[TMP11:%.*]] = extractelement <8 x float> %a0, i32 5
; CHECK-NEXT: [[TMP12:%.*]] = fadd float [[TMP10]], [[TMP11]]
; CHECK-NEXT: [[TMP13:%.*]] = extractelement <8 x float> %a0, i32 6
; CHECK-NEXT: [[TMP14:%.*]] = fadd float [[TMP12]], [[TMP13]]
; CHECK-NEXT: [[TMP15:%.*]] = extractelement <8 x float> %a0, i32 7
; CHECK-NEXT: [[TMP16:%.*]] = fadd float [[TMP14]], [[TMP15]]
; CHECK-NEXT: ret float [[TMP16]]
  %res = call float @llvm.vector.reduce.fadd.v8f32(float -0.000000e+00, <8 x float> %a0)
  ret float %res
}

declare i64 @llvm.vector.reduce.add.v2i64(<2 x i64>)
declare i32 @llvm.vector.reduce.add.v8i32(<8 x i32>)
declare i32 @llvm.vector.reduce.xor.v8i32(<8 x i32>)
declare i32 @llvm.vector.reduce.mul.v9i32(<9 x i32>)
declare i32 @llvm.vector.reduce.or.v7i32(<7 x i32>)
declare float @llvm.vector.reduce.fadd.v8f32(float, <8 x float>)

!0 = !{}
