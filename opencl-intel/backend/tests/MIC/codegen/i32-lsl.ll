; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF


target datalayout = "e-p:64:64"


define i32 @shiftright1(i32 %a, i32 %b) nounwind readnone ssp {
entry:
; KNF: shll
;
  %shl = shl i32 %a, %b
  ret i32 %shl
}

define i32 @shiftright3(i32 %a, i32* nocapture %b) nounwind readonly ssp {
entry:
; KNF: shll 

  %tmp2 = load i32* %b, align 64
  %shl = shl i32 %a, %tmp2
  ret i32 %shl
}


define i32 @shiftright2(i32* nocapture %a, i32 %b) nounwind readonly ssp {
entry:
; KNF: shll 
;

  %tmp1 = load i32* %a, align 64
  %shl = shl i32 %tmp1, %b
  ret i32 %shl
}