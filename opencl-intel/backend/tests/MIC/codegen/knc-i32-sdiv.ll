; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;
;
;

target datalayout = "e-p:64:64"

define i32 @sdiv(i32 %a, i32 %b) nounwind readnone ssp {
entry:
; KNF: idiv
;

  %sdiv =  sdiv i32 %a, %b                        ; <i32> [#uses=1]
  ret i32 %sdiv
}
