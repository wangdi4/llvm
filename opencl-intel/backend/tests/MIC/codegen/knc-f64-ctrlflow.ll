; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
;

target datalayout = "e-p:64:64"

@gf = common global double 0.000000e+000, align 8

define void @test(double %a, double %b) nounwind readnone ssp {
entry:
; KNC: vcmpnltpd %zmm0, %zmm1, %k0{%k1}
; KNC: jknzd
; KNC-NOT: testq
  %cmp = fcmp ogt double %a, %b
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store double 1.000000e+000, double* @gf, align 8
  ret void

if.end:                                           ; preds = %entry
  ret void
}
