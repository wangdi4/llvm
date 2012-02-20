; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
;

target datalayout = "e-p:64:64"

define i8 @xor(i8 %a, i8 %b) nounwind readnone ssp {
entry:
; KNF: xorl 
;

  %xor =  xor i8 %a, %b                        ; <i8> [#uses=1]
  ret i8 %xor
}
