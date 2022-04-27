; Check to see that sincospi and sincospif functions are translated to the high accuracy svml variant.

; RUN: opt -enable-new-pm=0 -vector-library=SVML -iml-trans -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: @vector_sincospif
; CHECK: call svml_cc { <4 x float>, <4 x float> } @__svml_sincospif4_ha
; CHECK: ret
define <4 x float> @vector_sincospif(<4 x float> %input, <4 x float>* nocapture %vcos) #0 {
entry:
  %0 = tail call svml_cc { <4 x float>, <4 x float> } @__svml_sincospif4(<4 x float> %input)
  %1 = extractvalue { <4 x float>, <4 x float> } %0, 1
  store <4 x float> %1, <4 x float>* %vcos, align 32
  %2 = extractvalue { <4 x float>, <4 x float> } %0, 0
  ret <4 x float> %2
}

; CHECK-LABEL: @vector_sincospi
; CHECK: call svml_cc { <2 x double>, <2 x double> } @__svml_sincospi2_ha
; CHECK: ret
define <2 x double> @vector_sincospi(<2 x double> %input, <2 x double>* nocapture %vcos) #0 {
entry:
  %0 = tail call svml_cc { <2 x double>, <2 x double> } @__svml_sincospi2(<2 x double> %input)
  %1 = extractvalue { <2 x double>, <2 x double> } %0, 1
  store <2 x double> %1, <2 x double>* %vcos, align 32
  %2 = extractvalue { <2 x double>, <2 x double> } %0, 0
  ret <2 x double> %2
}

; Function Attrs: argmemonly nounwind
declare { <4 x float>, <4 x float> } @__svml_sincospif4(<4 x float>) #1
declare { <2 x double>, <2 x double> } @__svml_sincospi2(<2 x double>) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

