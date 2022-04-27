; Check that both SVML function and scalar libm function calls are translated to legalized versions when -imf-arch-consistency is enabled.

; RUN: opt -enable-new-pm=0 -vector-library=SVML -S -iml-trans < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; The calls in this function have imf attributes and are libm functions.
; These should be transalted to correct variants.
define void @f1(double %c) {
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #2
  %scal_call = call double @exp(double %c) #2
  ret void
}
; CHECK-LABEL: @f1
; CHECK: call svml_cc <2 x double> @__svml_exp2_br
; CHECK: call double @__bwr_exp

; The calls in this function are either libm functions without imf attributes or non-libm functions.
; These should not be translated.
define void @f2(double %c) {
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #1
  %scal_call = call double @exp(double %c) #1
  %not_libm = call double @g(double %c)
  ret void
}
; CHECK-LABEL: @f2
; CHECK-NOT: call svml_cc <2 x double> @__svml_exp2_br
; CHECK-NOT: call double @__bwr_exp
; CHECK: call svml_cc <2 x double> @__svml_exp2_ha
; CHECK: call double @exp
; CHECK: call double @g


declare double @exp(double)
declare <4 x double> @__svml_exp4(<4 x double>)
declare double @g(double)

attributes #1 = { nounwind }
attributes #2 = { nounwind "imf-arch-consistency"="true" }
