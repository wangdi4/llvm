; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

@gf = common global double 0.000000e+000, align 8

define void @test(double %a, double %b) nounwind readnone ssp {
entry:
; KNF: vcmppd    {nlt}, %v0, %v1, %k0{%k1}
; KNF: vkortest  %k0, %k0
; KNF: jne
;
; KNC: vcmpnltpd %zmm0, %zmm1, %k0{%k1}
; KNC: kmovd %k0, [[R1:%[a-z]+]]
; KNC: testq [[R1]], [[R1]]
; KNC: jne
  %cmp = fcmp ogt double %a, %b
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store double 1.000000e+000, double* @gf, align 8
  ret void

if.end:                                           ; preds = %entry
  ret void
}
