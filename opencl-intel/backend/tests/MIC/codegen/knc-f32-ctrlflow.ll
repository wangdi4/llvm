; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
;

target datalayout = "e-p:64:64"

@gf = common global float 0.000000e+000, align 4

define void @test(float %a, float %b) nounwind readnone ssp {
entry:
; KNC: vcmpnltps %zmm0, %zmm1, %k0{%k1}
; KNC: jknzd
; KNC-NOT: testq
  %cmp = fcmp ogt float %a, %b
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store float 1.000000e+000, float* @gf, align 4
  ret void

if.end:                                           ; preds = %entry
  ret void
}
