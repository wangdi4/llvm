; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;
;


target datalayout = "e-p:64:64"

define i32 @and(i32 %a, i32 %b) nounwind readnone ssp {
entry:
; KNF: andl 
;

  %and =  and i32 %a, %b                        ; <i8> [#uses=1]
  ret i32 %and
}
