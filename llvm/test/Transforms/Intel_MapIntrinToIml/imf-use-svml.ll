; Check that imf-use-svml ensures SVML usage, regardless of whether fast math
; is present.

; RUN: opt -enable-new-pm=0 -vector-library=SVML -S -iml-trans < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define double @scalar(double %c) {
  %ret = call double @exp(double %c) #1
  ret double %ret
}
; CHECK-LABEL: @scalar
; CHECK: [[INSERT:%.*]] = insertelement <1 x double> undef, double %{{.*}}, i32 0
; CHECK: [[CALL:%.*]] = call svml_cc <1 x double> @__svml_exp1(<1 x double> [[INSERT]])
; CHECK: [[EXTRACT:%.*]] = extractelement <1 x double> [[CALL]], i32 0
; CHECK: ret double [[EXTRACT]]

define <4 x float> @vector_fmf(<4 x float> %A) #0 {
entry:
  %0 = tail call fast svml_cc <4 x float> @__svml_sinf4(<4 x float> %A) #1
  ret <4 x float> %0
}

; CHECK-LABEL: @vector_fmf
; CHECK: [[VCALL:%.*]] = call fast svml_cc <4 x float> @__svml_sinf4(<4 x float> %A)
; CHECK: ret <4 x float> [[VCALL]]

define <4 x float> @vector_nofmf(<4 x float> %A) #0 {
entry:
  %0 = tail call svml_cc <4 x float> @__svml_sinf4(<4 x float> %A) #1
  ret <4 x float> %0
}

; CHECK-LABEL: @vector_nofmf
; CHECK: [[VCALL:%.*]] = call svml_cc <4 x float> @__svml_sinf4(<4 x float> %A)
; CHECK: ret <4 x float> [[VCALL]]

declare double @exp(double)
declare <4 x float> @__svml_sinf4(<4 x float>)

attributes #1 = { nounwind "imf-use-svml"="true" }
