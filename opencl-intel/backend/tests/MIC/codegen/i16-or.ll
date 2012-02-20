; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define i16 @or(i16 %a, i16 %b) nounwind readnone ssp {
entry:
; KNF: orl 
;

  %or =  or i16 %a, %b                        ; <i16> [#uses=1]
  ret i16 %or
}
