; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

%0 = type { float, float, float }

define %0 @"vsub___s[_u_UniformVector]<UfUfUf>s[_u_UniformVector]<UfUfUf>"(%0 %a, %0 %b, <16 x i1> %__mask) nounwind readnone alwaysinline {
entry:
  ; KNF: ret
  %a26 = extractvalue %0 %a, 0
  %a27 = extractvalue %0 %a, 1
  %a28 = extractvalue %0 %a, 2
  %b23 = extractvalue %0 %b, 0
  %b24 = extractvalue %0 %b, 1
  %b25 = extractvalue %0 %b, 2
  %binop = fsub float %a26, %b23
  %binop11 = fsub float %a27, %b24
  %binop17 = fsub float %a28, %b25
  %insert = insertvalue %0 undef, float %binop, 0
  %insert20 = insertvalue %0 %insert, float %binop11, 1
  %insert22 = insertvalue %0 %insert20, float %binop17, 2
  ret %0 %insert22
}
