; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;
;
;

target datalayout = "e-p:64:64"

define i64 @sub(i64 %a, i64 %b) nounwind readnone ssp {
entry:
; KNF: subq
;

  %sub =  sub i64 %a, %b                        ; <i64> [#uses=1]
  ret i64 %sub
}
