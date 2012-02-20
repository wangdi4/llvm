; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;


target datalayout = "e-p:64:64"

define i64 @mul(i64 %a, i64 %b) nounwind readnone ssp {
entry:
; KNF: imulq
;

  %mul =  mul i64 %a, %b                        ; <i64> [#uses=1]
  ret i64 %mul
}

define i64 @mul_imm(i64 %a, i64 %b, i64 %c) nounwind readnone ssp {
entry:
; KNF: imul
;
  %mul0 =  mul nsw i64 %a, 144                        
  %mul1 =  mul nsw i64 %b, 134                        
  %mul2 =  mul nsw i64 %c, 14
  %mul3 =  sdiv i64 %mul0, %mul1
  %mul4 =  mul nsw i64 %mul3, %mul2                        
  ret i64 %mul4
}
