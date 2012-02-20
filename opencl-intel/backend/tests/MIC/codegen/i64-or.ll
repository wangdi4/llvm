; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF


target datalayout = "e-p:64:64"

define i64 @or(i64 %a, i64 %b) nounwind readnone ssp {
entry:
; KNF: orq 
;

  %or =  or i64 %a, %b                        ; <i64> [#uses=1]
  ret i64 %or
}

define i64 @or_cc(i64 %a, i64 %b) nounwind readnone {
entry:
; KNF: or 
;
  %or1 = or i64 %a, %b
  %or2 = trunc i64 %or1 to i32
  %or3 = icmp eq i32 %or2, 0
  br i1 %or3, label %t1, label %t2

t1:
  ret i64 12345
t2:
  ret i64 54321
}
