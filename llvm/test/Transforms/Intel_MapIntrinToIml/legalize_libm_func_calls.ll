; Check that both SVML function and scalar libm function calls are translated to legalized versions when -imf-arch-consistency is enabled.

; RUN: opt -bugpoint-enable-legacy-pm -vector-library=SVML -S -iml-trans < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,SVML
; RUN: opt -bugpoint-enable-legacy-pm -S -iml-trans < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,NOSVML

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; The calls in this function have imf attributes and are libm functions.
; These should be transalted to correct variants.
define void @f1(double %c) {
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call fast svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #2
  %scal_call = call double @exp(double %c) #2
  ret void
}
; CHECK-LABEL: @f1
; SVML: call fast svml_cc <2 x double> @__svml_exp2_br
; NOSVML-NOT: call fast svml_cc <2 x double> @__svml_exp2_br
; CHECK: call double @__bwr_exp

; The calls in this function are either libm functions without imf attributes or non-libm functions.
; These should not be translated.
define void @f2(double %c) {
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call fast svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #1
  %scal_call = call double @exp(double %c) #1
  %not_libm = call double @g(double %c)
  ret void
}
; CHECK-LABEL: @f2
; CHECK-NOT: call fast svml_cc <2 x double> @__svml_exp2_br
; CHECK-NOT: call double @__bwr_exp
; SVML: call fast svml_cc <2 x double> @__svml_exp2(
; NOSVML-NOT: call fast svml_cc <2 x double> @__svml_exp2(
; CHECK: call double @exp
; CHECK: call double @g

; Intrinsics decorated with IMF attributes should be translated to correct
; variants.
define void @f3(double %a, float %b, half %c) {
  %f64_scal_call = call double @exp(double %a)
  %f64_scal_call_br = call double @exp(double %a) #2
  %f64_scal_call_svml = call double @exp(double %a) #3
  %f64_scal_intrin_call = call double @llvm.exp.f64(double %a)
  %f64_scal_intrin_call_br = call double @llvm.exp.f64(double %a) #2
  %f64_scal_intrin_call_svml = call double @llvm.exp.f64(double %a) #3

  %f32_scal_call = call float @expf(float %b)
  %f32_scal_call_br = call float @expf(float %b) #2
  %f32_scal_call_svml = call float @expf(float %b) #3
  %f32_scal_intrin_call = call float @llvm.exp.f32(float %b)
  %f32_scal_intrin_call_br = call float @llvm.exp.f32(float %b) #2
  %f32_scal_intrin_call_svml = call float @llvm.exp.f32(float %b) #3

  %f16_scal_call = call half @expf16(half %c)
  %f16_scal_call_br = call half @expf16(half %c) #2
  %f16_scal_call_svml = call half @expf16(half %c) #3
  %f16_scal_intrin_call = call half @llvm.exp.f16(half %c)
  %f16_scal_intrin_call_br = call half @llvm.exp.f16(half %c) #2
  %f16_scal_intrin_call_svml = call half @llvm.exp.f16(half %c) #3
  ret void
}
; CHECK-LABEL: @f3
; CHECK: call double @exp(
; CHECK: call double @__bwr_exp(
; SVML: call svml_cc <1 x double> @__svml_exp1(
; NOSVML-NOT: call svml_cc <1 x double> @__svml_exp1(
; CHECK: call double @llvm.exp.f64(
; CHECK: call double @__bwr_exp(
; SVML: call svml_cc <1 x double> @__svml_exp1(
; NOSVML-NOT: call svml_cc <1 x double> @__svml_exp1(

; CHECK: call float @expf(
; CHECK: call float @__bwr_expf(
; SVML: call svml_cc <1 x float> @__svml_expf1(
; NOSVML-NOT: call svml_cc <1 x float> @__svml_expf1(
; CHECK: call float @llvm.exp.f32(
; CHECK: call float @__bwr_expf(
; SVML: call svml_cc <1 x float> @__svml_expf1(
; NOSVML-NOT: call svml_cc <1 x float> @__svml_expf1(

; FP16 functions currently have no BWR variant.
; TODO: libimf_attr is not working correctly with FP16 + imf-use-svml
; CHECK: call half @expf16(
; CHECK: call half @expf16(
; CHECK: call half @expf16(
; CHECK: call half @llvm.exp.f16(
; CHECK: call half @llvm.exp.f16(
; CHECK: call half @llvm.exp.f16(

; lround is special in it having an integer return type. Check that we can
; handle it as well.
define void @test_lround(double %a, float %b, half %c) {
  %f64_scal_call = call i64 @lround(double %a)
  %f64_scal_call_br = call i64 @lround(double %a) #2
  %f64_scal_call_svml = call i64 @lround(double %a) #3
  %f64_scal_intrin_call = call i64 @llvm.lround.i64.f64(double %a)
  %f64_scal_intrin_call_br = call i64 @llvm.lround.i64.f64(double %a) #2
  %f64_scal_intrin_call_svml = call i64 @llvm.lround.i64.f64(double %a) #3

  %f32_scal_call = call i64 @lroundf(float %b)
  %f32_scal_call_br = call i64 @lroundf(float %b) #2
  %f32_scal_call_svml = call i64 @lroundf(float %b) #3
  %f32_scal_intrin_call = call i64 @llvm.lround.i64.f32(float %b)
  %f32_scal_intrin_call_br = call i64 @llvm.lround.i64.f32(float %b) #2
  %f32_scal_intrin_call_svml = call i64 @llvm.lround.i64.f32(float %b) #3

  %f16_scal_call = call i64 @lroundf16(half %c)
  %f16_scal_call_br = call i64 @lroundf16(half %c) #2
  %f16_scal_call_svml = call i64 @lroundf16(half %c) #3
  %f16_scal_intrin_call = call i64 @llvm.lround.i64.f16(half %c)
  %f16_scal_intrin_call_br = call i64 @llvm.lround.i64.f16(half %c) #2
  %f16_scal_intrin_call_svml = call i64 @llvm.lround.i64.f16(half %c) #3
  ret void
}

; CHECK-LABEL: @test_lround
; CHECK: call i64 @lround(
; CHECK: call i64 @__bwr_lround(
; CHECK: call i64 @lround(
; CHECK: call i64 @llvm.lround.i64.f64(
; CHECK: call i64 @__bwr_lround(
; CHECK: call i64 @llvm.lround.i64.f64(

; CHECK: call i64 @lroundf(
; CHECK: call i64 @__bwr_lroundf(
; CHECK: call i64 @lroundf(
; CHECK: call i64 @llvm.lround.i64.f32(
; CHECK: call i64 @__bwr_lroundf(
; CHECK: call i64 @llvm.lround.i64.f32(

; CHECK: call i64 @lroundf16(
; CHECK: call i64 @lroundf16(
; CHECK: call i64 @lroundf16(
; CHECK: call i64 @llvm.lround.i64.f16(
; CHECK: call i64 @llvm.lround.i64.f16(
; CHECK: call i64 @llvm.lround.i64.f16(

declare double @exp(double)
declare double @llvm.exp.f64(double)
declare float @expf(float)
declare float @llvm.exp.f32(float)
declare half @expf16(half)
declare half @llvm.exp.f16(half)

declare i64 @lround(double)
declare i64 @llvm.lround.i64.f64(double)
declare i64 @lroundf(float)
declare i64 @llvm.lround.i64.f32(float)
declare i64 @lroundf16(half)
declare i64 @llvm.lround.i64.f16(half)

declare <4 x double> @__svml_exp4(<4 x double>)
declare double @g(double)

attributes #1 = { nounwind }
attributes #2 = { nounwind "imf-arch-consistency"="true" }
attributes #3 = { nounwind "imf-use-svml"="true" }
