; RUN: opt -pre-isel-intrinsic-lowering -S -o - %s | FileCheck %s

; This test verifies that the PreISelIntrsicLowering pass correctly lowers
; the intel.honor.fcmp intrinsic and preserves appropriate fast-math flags.
; In cases where the result is known, the lowering may produce a constant.
; Whether or not it does depends on the constant folder. This test should
; be kept up-to-date with actual behavior and the correctness of the
; behavior should be verified when the test is updated.

; CHECK: @test_self_eq_f32
; CHECK: fcmp reassoc ninf nsz arcp contract afn oeq float %x, %x
define i32 @test_self_eq_f32(float %x) local_unnamed_addr {
entry:
  %cmp = tail call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f32(float %x, float %x, metadata !"oeq")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; CHECK: @test_self_eq_f64
; CHECK: fcmp reassoc ninf nsz arcp contract afn oeq double %x, %x
define i32 @test_self_eq_f64(double %x) local_unnamed_addr {
entry:
  %cmp = tail call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64(double %x, double %x, metadata !"oeq")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; CHECK: @test_self_ne_f32
; CHECK: fcmp reassoc ninf nsz arcp contract afn une float %x, %x
define i32 @test_self_ne_f32(float %x) local_unnamed_addr {
entry:
  %cmp = tail call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f32(float %x, float %x, metadata !"une")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; CHECK: @test_self_ne_f64
; CHECK: fcmp reassoc ninf nsz arcp contract afn une double %x, %x
define i32 @test_self_ne_f64(double %x) local_unnamed_addr {
entry:
  %cmp = tail call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64(double %x, double %x, metadata !"une")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; Note: It would also be acceptable for the next four cases to be folded to
;       false but that isn't currently happening. See test_const_nan_eq_f64 for
;       details on why folding would be OK.

; CHECK: @test_nan_eq_f32
; CHECK: fcmp reassoc ninf nsz arcp contract afn oeq float %x, 0x7FF8000000000000
define i32 @test_nan_eq_f32(float %x) local_unnamed_addr {
entry:
  %cmp = tail call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f32(float %x, float 0x7FF8000000000000, metadata !"oeq")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; CHECK: @test_nan_eq_f64
; CHECK: fcmp reassoc ninf nsz arcp contract afn oeq double %x, 0x7FF8000000000000
define i32 @test_nan_eq_f64(double %x) local_unnamed_addr {
entry:
  %cmp = tail call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64(double %x, double 0x7FF8000000000000, metadata !"oeq")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; CHECK: @test_nan_ne_f32
; CHECK: fcmp reassoc ninf nsz arcp contract afn une float %x, 0x7FF8000000000000
define i32 @test_nan_ne_f32(float %x) local_unnamed_addr {
entry:
  %cmp = tail call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f32(float %x, float 0x7FF8000000000000, metadata !"une")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; CHECK: @test_nan_ne_f64
; CHECK: fcmp reassoc ninf nsz arcp contract afn une double %x, 0x7FF8000000000000
define i32 @test_nan_ne_f64(double %x) local_unnamed_addr {
entry:
  %cmp = tail call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64(double %x, double 0x7FF8000000000000, metadata !"une")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; CHECK: @test_vec_eq_f32
; CHECK: fcmp reassoc ninf nsz arcp contract afn oeq <4 x float> %x, %y
define <4 x i32> @test_vec_eq_f32(<4 x float> %x, <4 x float> %y) local_unnamed_addr {
entry:
  %cmp = tail call reassoc ninf nsz arcp contract afn <4 x i1> @llvm.intel.honor.fcmp.v4f32(<4 x float> %x, <4 x float> %y, metadata !"oeq")
  %sext = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sext
}

; CHECK: @test_vec_gt_f32
; CHECK: fcmp reassoc ninf nsz arcp contract afn ogt <4 x float> %x, %y
define <4 x i32> @test_vec_gt_f32(<4 x float> %x, <4 x float> %y) local_unnamed_addr {
entry:
  %cmp = tail call reassoc ninf nsz arcp contract afn <4 x i1> @llvm.intel.honor.fcmp.v4f32(<4 x float> %x, <4 x float> %y, metadata !"ogt")
  %sext = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sext
}

; CHECK: @test_vec_eq_f64
; CHECK: fcmp reassoc ninf nsz arcp contract afn oeq <2 x double> %x, %y
define <2 x i64> @test_vec_eq_f64(<2 x double> %x, <2 x double> %y) local_unnamed_addr {
entry:
  %cmp = tail call reassoc ninf nsz arcp contract afn <2 x i1> @llvm.intel.honor.fcmp.v2f64(<2 x double> %x, <2 x double> %y, metadata !"oeq")
  %sext = sext <2 x i1> %cmp to <2 x i64>
  ret <2 x i64> %sext
}

; CHECK: @test_vec_gt_f64
; CHECK: fcmp reassoc ninf nsz arcp contract afn ogt <2 x double> %x, %y
define <2 x i64> @test_vec_gt_f64(<2 x double> %x, <2 x double> %y) local_unnamed_addr {
entry:
  %cmp = tail call reassoc ninf nsz arcp contract afn <2 x i1> @llvm.intel.honor.fcmp.v2f64(<2 x double> %x, <2 x double> %y, metadata !"ogt")
  %sext = sext <2 x i1> %cmp to <2 x i64>
  ret <2 x i64> %sext
}

; This case can be constant folded safely since the operands are constant.
; The semantics of the intrinsic require that the operands are never assumed
; not to be NaN, but if one or both operands are known to be NaN, the result
; can be folded to false.
; CHECK: @test_const_nan_eq_f64
; CHECK-NEXT: entry:
; CHECK-NEXT: [[CONV:%.*]] = zext i1 false to i32
; CHECK-NEXT: ret i32 [[CONV]]
@g3 = local_unnamed_addr constant double 3.000000e+00, align 8
define i32 @test_const_nan_eq_f64() local_unnamed_addr {
entry:
  %cmp = tail call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64(double 3.000000e+00, double 0x7FF8000000000000, metadata !"oeq")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; CHECK: @test_const_self_eq_f64
; CHECK-NEXT: entry:
; CHECK-NEXT: [[CONV:%.*]] = zext i1 true to i32
; CHECK-NEXT: ret i32 [[CONV]]
define i32 @test_const_self_eq_f64() local_unnamed_addr {
entry:
  %cmp = tail call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64(double 0.000000e+00, double 0.000000e+00, metadata !"oeq")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; CHECK: @test_const_nan_self_eq_f64
; CHECK-NEXT: entry:
; CHECK-NEXT: [[CONV:%.*]] = zext i1 false to i32
; CHECK-NEXT: ret i32 [[CONV]]
define i32 @test_const_nan_self_eq_f64() local_unnamed_addr {
entry:
  %cmp = tail call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64(double 0x7FF8000000000000, double 0x7FF8000000000000, metadata !"oeq")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

declare i1 @llvm.intel.honor.fcmp.f32(float, float, metadata)
declare i1 @llvm.intel.honor.fcmp.f64(double, double, metadata)
declare <4 x i1> @llvm.intel.honor.fcmp.v4f32(<4 x float>, <4 x float>, metadata)
declare <2 x i1> @llvm.intel.honor.fcmp.v2f64(<2 x double>, <2 x double>, metadata)
