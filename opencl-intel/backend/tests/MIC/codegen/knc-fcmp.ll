; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc | FileCheck %s
;

define <2 x i32> @fcmp_2(<2 x float> %a, <2 x float> %b) nounwind uwtable readnone {
; CHECK: fcmp_2:
  %1 = fcmp oeq <2 x float> %a, %b
  %2 = sext <2 x i1> %1 to <2 x i32>
  ret <2 x i32> %2
}

define <8 x i32> @fcmp_8(<8 x float> %a, <8 x float> %b) nounwind uwtable readnone {
; CHECK: fcmp_8:
  %1 = fcmp oeq <8 x float> %a, %b
  %2 = sext <8 x i1> %1 to <8 x i32>
  ret <8 x i32> %2
}
