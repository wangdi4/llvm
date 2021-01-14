; RUN: sycl-post-link -ompoffload-link-entries -ompoffload-explicit-simd --ir-output-only %s -S -o - | FileCheck %s

define dso_local spir_kernel i64 @test_v2i64(<2 x i64> %a0) {
; CHECK: [[TMP1:%.*]] = extractelement <2 x i64> %a0, i32 0
; CHECK-NEXT: [[TMP2:%.*]] = extractelement <2 x i64> %a0, i32 1
; CHECK-NEXT: [[RES:%.*]] = add i64 [[TMP1]], [[TMP2]]
  %1 = call i64 @llvm.vector.reduce.add.v2i64(<2 x i64> %a0)
  ret i64 %1
}

define dso_local spir_kernel i32 @test_v8i32(<8 x i32> %a0) {
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

define dso_local spir_kernel i32 @test_xor_v8i32(<8 x i32> %a0) {
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

define dso_local spir_kernel i32 @test_non_pow2(<9 x i32> %a0) {
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

define dso_local spir_kernel i32 @test_non_pow2_2(<7 x i32> %a0) {
; CHECK: [[TMP1:%.*]] = call <2 x i32> @llvm.genx.rdregioni.v2i32.v7i32.i16
; CHECK-NEXT: [[TMP2:%.*]] = call <2 x i32> @llvm.genx.rdregioni.v2i32.v7i32.i16
; CHECK-NEXT: [[TMP3:%.*]] = or <2 x i32> [[TMP1]], [[TMP2]]
; CHECK-NEXT: [[TMP4:%.*]] = call <2 x i32> @llvm.genx.rdregioni.v2i32.v2i32.i16(<2 x i32> [[TMP3]]
; CHECK-NEXT: [[REDUCE_ACROSS:%.*]] = call <2 x i32> @llvm.genx.rdregioni.v2i32.v7i32.i16(<7 x i32> %a0, i32 0, i32 2, i32 1, i16 16, i32 0)
; CHECK-NEXT: [[TMP5:%.*]] = or <2 x i32> [[TMP4]], [[REDUCE_ACROSS]]
  %1 = call i32 @llvm.vector.reduce.or.v7i32(<7 x i32> %a0)
  ret i32 %1
}

declare i64 @llvm.vector.reduce.add.v2i64(<2 x i64>)
declare i32 @llvm.vector.reduce.add.v8i32(<8 x i32>)
declare i32 @llvm.vector.reduce.xor.v8i32(<8 x i32>)
declare i32 @llvm.vector.reduce.mul.v9i32(<9 x i32>)
declare i32 @llvm.vector.reduce.or.v7i32(<7 x i32>)
