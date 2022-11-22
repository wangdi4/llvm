; Check that both SVML function and scalar libm function calls are translated to legalized versions when -imf-arch-consistency is enabled.

; RUN: opt -enable-new-pm=0 -vector-library=SVML -S -iml-trans < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define double @f(double %c) {
  %ret = call double @exp(double %c) #1
  ret double %ret
}
; CHECK-LABEL: @f
; CHECK: [[INSERT:%.*]] = insertelement <1 x double> undef, double %{{.*}}, i32 0
; CHECK: [[CALL:%.*]] = call svml_cc <1 x double> @__svml_exp1_ha(<1 x double> [[INSERT]])
; CHECK: [[EXTRACT:%.*]] = extractelement <1 x double> [[CALL]], i32 0
; CHECK: ret double [[EXTRACT]]

declare double @exp(double)

attributes #1 = { nounwind "imf-use-svml"="true" }
