; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;


target datalayout = "e-p:64:64"


define i64 @shiftright1(i64 %a, i64 %b) nounwind readnone ssp {
entry:
; KNF: shrq
;
  %lshr = lshr i64 %a, %b
  ret i64 %lshr
}

define i64 @shiftright3(i64 %a, i64* nocapture %b) nounwind readonly ssp {
entry:
; KNF: shrq 

  %tmp2 = load i64* %b, align 64
  %lshr = lshr i64 %a, %tmp2
  ret i64 %lshr
}


define i64 @shiftright2(i64* nocapture %a, i64 %b) nounwind readonly ssp {
entry:
; KNF: shrq 
;

  %tmp1 = load i64* %a, align 64
  %lshr = lshr i64 %tmp1, %b
  ret i64 %lshr
}