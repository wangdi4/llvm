; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 


;


target datalayout = "e-p:64:64"


define i8 @shiftright3(i8 %a, i8* nocapture %b) nounwind readonly ssp {
entry:
; KNF: sarb 

  %tmp2 = load i8* %b, align 64
  %shr = ashr i8 %a, %tmp2
  ret i8 %shr
}

