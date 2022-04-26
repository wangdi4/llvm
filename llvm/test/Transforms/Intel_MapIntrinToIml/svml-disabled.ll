; Check that MapIntrinToIml doesn't touch calls to function with __svml prefix
; when SVML is not enabled;
; RUN: opt -enable-new-pm=0 -vector-library=none -iml-trans -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: @test_sinf8
; CHECK: call fast svml_cc <8 x float> @__svml_sinf8(

define <8 x float> @test_sinf8(<8 x float> %A) #0 {
entry:
  %0 = tail call fast svml_cc <8 x float> @__svml_sinf8(<8 x float> %A)
  ret <8 x float> %0
}

; CHECK-LABEL: @__svml_acos1_e7
; CHECK: call fast svml_cc <2 x double> @__svml_acos2_e7(

define double @__svml_acos1_e7(double %A) #0 {
entry:
  %0 = insertelement <2 x double> undef, double %A, i32 0
  %1 = call fast svml_cc <2 x double> @__svml_acos2_e7(<2 x double> %0)
  %2 = extractelement <2 x double> %0, i32 0
  ret double %2
}

declare <8 x float> @__svml_sinf8(<8 x float>) #3
declare <2 x double> @__svml_acos2_e7(<2 x double>) #3

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }
