; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc  | FileCheck %s

;
;


target datalayout = "e-p:64:64"

define i32 @and(i32 %a, i32 %b) nounwind readnone ssp {
entry:
; CHECK: andl 

  %and =  and i32 %a, %b                        ; <i8> [#uses=1]
  ret i32 %and
}

define i32 @andi(i32 %a, i32 %b) nounwind readnone ssp {
entry:
; CHECK: andl $5

  %and =  and i32 %a, 5                        ; <i8> [#uses=1]
  ret i32 %and
}
