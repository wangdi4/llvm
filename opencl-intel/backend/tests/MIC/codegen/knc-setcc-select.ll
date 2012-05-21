; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

define float @test(float %a, float %b) nounwind uwtable readnone {
  %sel = fcmp olt float %a, %b
  %res = select i1 %sel, float 0.000000e+00, float 1.000000e+00
  ret float %res
}


