; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s --check-prefix=KNC
;

target datalayout = "e-p:64:64"

define double @cvt(i32 %a) nounwind readnone ssp {
entry:
; KNF: vcvtpi2pd
; 
; KNC: vcvtdq2pd -8(%rsp){1to8}, %zmm0{%k{{[1-9]}}}
  %conv = sitofp i32 %a to double
  ret double %conv
}

@g = common global i32 0, align 4

define double @cvtm() nounwind readnone ssp {
entry:
  %i = load i32* @g
; KNF: vcvtpi2pd
;
; KNC: vcvtdq2pd g(%rip){1to8}, %zmm0{%k{{[1-9]}}}
  %conv = sitofp i32 %i to double
  ret double %conv
}
