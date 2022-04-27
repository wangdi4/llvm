; Check to see that __svml_sincosf4 is translated to the high accuracy svml variant.

; RUN: opt -enable-new-pm=0 -vector-library=SVML -iml-trans -S < %s | FileCheck %s

; CHECK-LABEL: @vector_foo
; CHECK: call svml_cc { <4 x float>, <4 x float> } @__svml_sincosf4_ha
; CHECK: ret

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define <4 x float> @vector_foo(<4 x float> %input, <4 x float>* nocapture %vcos) #0 {
entry:
  %0 = tail call svml_cc { <4 x float>, <4 x float> } @__svml_sincosf4(<4 x float> %input)
  %1 = extractvalue { <4 x float>, <4 x float> } %0, 1
  store <4 x float> %1, <4 x float>* %vcos, align 32
  %2 = extractvalue { <4 x float>, <4 x float> } %0, 0
  ret <4 x float> %2
}

; Function Attrs: argmemonly nounwind
declare { <4 x float>, <4 x float> } @__svml_sincosf4(<4 x float>) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
