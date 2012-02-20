; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;


target datalayout = "e-p:64:64"

define i64 @and(i64 %a, i64 %b) nounwind readnone ssp {
entry:
; KNF: andq 
;

  %and =  and i64 %a, %b                        ; <i8> [#uses=1]
  ret i64 %and
}
