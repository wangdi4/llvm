; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF


target datalayout = "e-p:64:64"


define i64 @shiftright1(i64 %a, i64 %b) nounwind readnone ssp {
entry:
; KNF: shlq
;
  %shl = shl i64 %a, %b
  ret i64 %shl
}

define i64 @shiftright3(i64 %a, i64* nocapture %b) nounwind readonly ssp {
entry:
; KNF: shlq 

  %tmp2 = load i64* %b, align 64
  %shl = shl i64 %a, %tmp2
  ret i64 %shl
}


define i64 @shiftright2(i64* nocapture %a, i64 %b) nounwind readonly ssp {
entry:
; KNF: shlq 
;

  %tmp1 = load i64* %a, align 64
  %shl = shl i64 %tmp1, %b
  ret i64 %shl
}