; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF


target datalayout = "e-p:64:64"


define i16 @shiftright1(i16 %a, i16 %b) nounwind readnone ssp {
entry:
; KNF: shrl 
;
  %shr = lshr i16 %a, %b
  ret i16 %shr
}

define i16 @shiftright3(i16 %a, i16* nocapture %b) nounwind readonly ssp {
entry:
; KNF: shrl

  %tmp2 = load i16* %b, align 64
  %shr = lshr i16 %a, %tmp2
  ret i16 %shr
}


define i16 @shiftright2(i16* nocapture %a, i16 %b) nounwind readonly ssp {
entry:
; KNF: shrl 


  %tmp1 = load i16* %a, align 64
  %shr = lshr i16 %tmp1, %b
  ret i16 %shr
}