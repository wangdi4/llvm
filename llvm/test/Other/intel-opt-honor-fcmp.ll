; RUN: opt -O2 -S < %s | FileCheck %s
; RUN: opt -O3 -S < %s | FileCheck %s

; This test verifies that intel.honor.fcmp instructions are handled correctly.
; In most cases that will mean simply leaving the intrinsic in place. That
; is currently the expected behavior for all cases in this test. A few cases
; could be constant folded as long as NaN values are respected.

; CHECK: @test_self_eq_f32
; CHECK: call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f32
define i32 @test_self_eq_f32(float %x) {
entry:
  %x.addr = alloca float, align 4
  store float %x, float* %x.addr, align 4
  %i = load float, float* %x.addr, align 4
  %i1 = load float, float* %x.addr, align 4
  %cmp = call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f32(float %i, float %i1, metadata !"oeq")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; CHECK: @test_self_eq_f64
; CHECK: call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64
define i32 @test_self_eq_f64(double %x) {
entry:
  %x.addr = alloca double, align 8
  store double %x, double* %x.addr, align 8
  %i = load double, double* %x.addr, align 8
  %i1 = load double, double* %x.addr, align 8
  %cmp = call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64(double %i, double %i1, metadata !"oeq")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; CHECK: @test_self_ne_f32
; CHECK: call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f32
define i32 @test_self_ne_f32(float %x) {
entry:
  %x.addr = alloca float, align 4
  store float %x, float* %x.addr, align 4
  %i = load float, float* %x.addr, align 4
  %i1 = load float, float* %x.addr, align 4
  %cmp = call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f32(float %i, float %i1, metadata !"une")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; CHECK: @test_self_ne_f64
; CHECK: call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64
define i32 @test_self_ne_f64(double %x) {
entry:
  %x.addr = alloca double, align 8
  store double %x, double* %x.addr, align 8
  %i = load double, double* %x.addr, align 8
  %i1 = load double, double* %x.addr, align 8
  %cmp = call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64(double %i, double %i1, metadata !"une")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; CHECK: @test_nan_eq_f32
; CHECK: call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f32
define i32 @test_nan_eq_f32(float %x) {
entry:
  %x.addr = alloca float, align 4
  store float %x, float* %x.addr, align 4
  %i = load float, float* %x.addr, align 4
  %cmp = call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f32(float %i, float 0x7FF8000000000000, metadata !"oeq")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; CHECK: @test_nan_eq_f64
; CHECK: call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64
define i32 @test_nan_eq_f64(double %x) {
entry:
  %x.addr = alloca double, align 8
  store double %x, double* %x.addr, align 8
  %i = load double, double* %x.addr, align 8
  %cmp = call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64(double %i, double 0x7FF8000000000000, metadata !"oeq")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; CHECK: @test_nan_ne_f32
; CHECK: call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f32
define i32 @test_nan_ne_f32(float %x) {
entry:
  %x.addr = alloca float, align 4
  store float %x, float* %x.addr, align 4
  %i = load float, float* %x.addr, align 4
  %cmp = call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f32(float %i, float 0x7FF8000000000000, metadata !"une")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; CHECK: @test_nan_ne_f64
; CHECK: call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64
define i32 @test_nan_ne_f64(double %x) {
entry:
  %x.addr = alloca double, align 8
  store double %x, double* %x.addr, align 8
  %i = load double, double* %x.addr, align 8
  %cmp = call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64(double %i, double 0x7FF8000000000000, metadata !"une")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; CHECK: @test_vec_eq_f32
; CHECK: call reassoc ninf nsz arcp contract afn <4 x i1> @llvm.intel.honor.fcmp.v4f32
define <4 x i32> @test_vec_eq_f32(<4 x float> %x, <4 x float> %y) {
entry:
  %x.addr = alloca <4 x float>, align 16
  %y.addr = alloca <4 x float>, align 16
  store <4 x float> %x, <4 x float>* %x.addr, align 16
  store <4 x float> %y, <4 x float>* %y.addr, align 16
  %i = load <4 x float>, <4 x float>* %x.addr, align 16
  %i1 = load <4 x float>, <4 x float>* %y.addr, align 16
  %cmp = call reassoc ninf nsz arcp contract afn <4 x i1> @llvm.intel.honor.fcmp.v4f32(<4 x float> %i, <4 x float> %i1, metadata !"oeq")
  %sext = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sext
}

; CHECK: @test_vec_gt_f32
; CHECK: call reassoc ninf nsz arcp contract afn <4 x i1> @llvm.intel.honor.fcmp.v4f32
define <4 x i32> @test_vec_gt_f32(<4 x float> %x, <4 x float> %y) {
entry:
  %x.addr = alloca <4 x float>, align 16
  %y.addr = alloca <4 x float>, align 16
  store <4 x float> %x, <4 x float>* %x.addr, align 16
  store <4 x float> %y, <4 x float>* %y.addr, align 16
  %i = load <4 x float>, <4 x float>* %x.addr, align 16
  %i1 = load <4 x float>, <4 x float>* %y.addr, align 16
  %cmp = call reassoc ninf nsz arcp contract afn <4 x i1> @llvm.intel.honor.fcmp.v4f32(<4 x float> %i, <4 x float> %i1, metadata !"ogt")
  %sext = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sext
}

; CHECK: @test_vec_eq_f64
; CHECK: call reassoc ninf nsz arcp contract afn <2 x i1> @llvm.intel.honor.fcmp.v2f64
define <2 x i64> @test_vec_eq_f64(<2 x double> %x, <2 x double> %y) {
entry:
  %x.addr = alloca <2 x double>, align 16
  %y.addr = alloca <2 x double>, align 16
  store <2 x double> %x, <2 x double>* %x.addr, align 16
  store <2 x double> %y, <2 x double>* %y.addr, align 16
  %i = load <2 x double>, <2 x double>* %x.addr, align 16
  %i1 = load <2 x double>, <2 x double>* %y.addr, align 16
  %cmp = call reassoc ninf nsz arcp contract afn <2 x i1> @llvm.intel.honor.fcmp.v2f64(<2 x double> %i, <2 x double> %i1, metadata !"oeq")
  %sext = sext <2 x i1> %cmp to <2 x i64>
  ret <2 x i64> %sext
}

; CHECK: @test_vec_gt_f64
; CHECK: call reassoc ninf nsz arcp contract afn <2 x i1> @llvm.intel.honor.fcmp.v2f64
define <2 x i64> @test_vec_gt_f64(<2 x double> %x, <2 x double> %y) {
entry:
  %x.addr = alloca <2 x double>, align 16
  %y.addr = alloca <2 x double>, align 16
  store <2 x double> %x, <2 x double>* %x.addr, align 16
  store <2 x double> %y, <2 x double>* %y.addr, align 16
  %i = load <2 x double>, <2 x double>* %x.addr, align 16
  %i1 = load <2 x double>, <2 x double>* %y.addr, align 16
  %cmp = call reassoc ninf nsz arcp contract afn <2 x i1> @llvm.intel.honor.fcmp.v2f64(<2 x double> %i, <2 x double> %i1, metadata !"ogt")
  %sext = sext <2 x i1> %cmp to <2 x i64>
  ret <2 x i64> %sext
}

; CHECK: @test_const_nan_eq_f64
; CHECK: call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64
@g3 = constant double 3.000000e+00, align 8
define i32 @test_const_nan_eq_f64(double %x) {
entry:
  %x.addr = alloca double, align 8
  store double %x, double* %x.addr, align 8
  %cmp = call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64(double 3.000000e+00, double 0x7FF8000000000000, metadata !"oeq")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; CHECK: @test_self_sub_eq0_f64
; CHECK: call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64
define i32 @test_self_sub_eq0_f64(double %x) {
entry:
  %x.addr = alloca double, align 8
  %t = alloca double, align 8
  store double %x, double* %x.addr, align 8
  %i = load double, double* %x.addr, align 8
  %i1 = load double, double* %x.addr, align 8
  %sub = fsub fast double %i, %i1
  store double %sub, double* %t, align 8
  %i2 = load double, double* %t, align 8
  %cmp = call reassoc ninf nsz arcp contract afn i1 @llvm.intel.honor.fcmp.f64(double %i2, double 0.000000e+00, metadata !"oeq")
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

declare i1 @llvm.intel.honor.fcmp.f32(float, float, metadata)
declare i1 @llvm.intel.honor.fcmp.f64(double, double, metadata)
declare <4 x i1> @llvm.intel.honor.fcmp.v4f32(<4 x float>, <4 x float>, metadata)
declare <2 x i1> @llvm.intel.honor.fcmp.v2f64(<2 x double>, <2 x double>, metadata)
