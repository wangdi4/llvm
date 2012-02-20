; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
target datalayout = "e-p:64:64"

define i32 @sdivrem(i32 %a, i32 %b) nounwind readnone ssp {
entry:
; KNF: div
  %div = sdiv i32 %a, %b
  %rem = srem i32 %a, %b 
  %res = add i32 %div, %rem 
  ret i32 %res
}
